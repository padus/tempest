//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/padus/tempest
//
// Description: Inter-process communication

#ifndef TEMPEST_IPC
#define TEMPEST_IPC

// Includes --------------------------------------------------------------------------------------------------------------------

#include "system.hpp"

// Source ----------------------------------------------------------------------------------------------------------------------

namespace tempest {

#define TEMPEST_IPC_NAME        "/tempest_ipc"

#define TEMPEST_IPC_PERM        (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define SHM_FAILED              ((void *) -1)

using namespace std;

class Ipc {
public:

  Ipc() {
    init_ = false;
    locked_ = false;

    Clear();
  }

  virtual ~Ipc() {
    Close();
  }

  error_t Initialize(const char* name, size_t size) {
    //
    // Return EPERM if IPC has already been initialized
    // or a system error in case of failed initialization
    //
    error_t err = EPERM;

    if (!init_ && !(err = Open(name, size))) init_ = true;
    
    return (err);
  }

  error_t Deinitialize(void) {
    //
    // Return EPERM if IPC has not already been initialized
    // or a system error in case of failed deinitialization
    //
    error_t err = EPERM;

    if (init_ && !(err = Close())) init_ = false;
    
    return (err);
  }

  error_t Acquire(void*& addr, time_t wait = -1) {
    //
    // If wait:
    //   0) try the semaphore; if it cannot be acquired immediately return EAGAIN 
    //   n) if the semaphore cannot be acquired within the n milliseconds wait, return ETIMEDOUT
    //  -1) wait indefinitely to acquire the semaphore   
    //
    // Return EPERM if IPC has not already been initialized or has already been acquired
    // or a system error in case of failed acquisition
    //
    error_t err = EPERM;
    addr = nullptr;

    if (init_ && !locked_ && !(err = Lock(wait))) {
      locked_ = true;
      addr = addr_;  
    }

    return (err);
  }

  error_t Release(void*& addr) {
    //
    // Return EPERM if IPC has not already been initialized or has not already been acquired
    // or a system error in case of failed acquisition
    //    
    error_t err = EPERM;

    if (init_ && locked_ && !(err = Unlock())) {
      locked_ = false;
      addr = nullptr;  
    }

    return (err);
  }

private:

  error_t Lock(time_t wait = -1) {
    //
    // Lock the shared memory semaphore
    //
    error_t err = 0;
    int ret;

    switch (wait) {
    case 0:
      ret = sem_trywait(sem_);
      break;      

    case -1:
      ret = sem_wait(sem_);
      break;

    default:
      struct timespec to;

      to.tv_sec = time(nullptr) + (wait / 1000);
      to.tv_nsec = (wait % 1000) * 1000;

      ret = sem_timedwait(sem_, &to);
    }

    if (ret == -1) err = errno;

    return (err);
  }

  error_t Unlock(void) {
    //
    // Unlock the shared memory semaphore    
    //
    error_t err = 0;

    if (sem_post(sem_) == -1) err = errno;

    return (err);
  }

  error_t Remove(void) {
    //
    // Remove a ghost shared memory segment (present but not attached to any process)
    // Return ENOENT if none were found
    //
    error_t err = 0;

    int id;

    if ((id = shmget(key_, 0, TEMPEST_IPC_PERM)) != -1) {
      // Found memory segment
      struct shmid_ds shm;

      if (shmctl(id, IPC_STAT, &shm) == -1) err = errno;
      else if (!shm.shm_nattch && shmctl(id, IPC_RMID, nullptr) == -1) err = errno;
    }
    else if (errno != ENOENT) err = errno;

    return (err);
  }
 
  error_t Open(const char* name, size_t size) {
    //
    // Initialize shared memory segment with semaphore to restrict access
    //
    error_t err = 0;

    key_ = hash<string>{}(name);

    if ((sem_ = sem_open(name, O_CREAT, TEMPEST_IPC_PERM, 1)) == SEM_FAILED) err = errno;
    else if (!(err = Lock())) {
      // Remove orphaned shared memory segment (if any)
      if (!(err = Remove())) {
        int id;

        if ((id = shmget(key_, size, IPC_CREAT | TEMPEST_IPC_PERM)) == -1) err = errno;
        else if ((addr_ = shmat(id, nullptr, 0)) == SHM_FAILED) err = errno;    
      }

      error_t lck = Unlock();
      if (lck && !err) err = lck;

    }

    return (err);
  }

  error_t Close(void) {
    //
    // Seinitialize shared memory segment and its associated semaphore
    //    
    error_t err = 0;

    if (sem_ != SEM_FAILED) {
      if (!(err = Lock())) {
        if (addr_ != SHM_FAILED && shmdt(addr_) == -1 && !err) err = errno;
        else err = Remove();

        error_t lck = Unlock();
        if (lck && !err) err = lck;
      } 

      if (sem_close(sem_) == -1 && !err) err = errno;
    }

    Clear();

    return (err);
  }

  error_t Clear(void) {

    key_ = -1;
    addr_ = SHM_FAILED;
    sem_ = SEM_FAILED;
     
    return (0);
  }

  bool init_;
  bool locked_;

  key_t key_;
  void* addr_;
  sem_t* sem_;
};

class Rpc: public Ipc {
public:

  enum Command: int {
    NONE = 0,
    STOP = 0,
    STATS = 1,
    VERSION = 2  
  };

  Rpc(): Ipc() {
    shm_ = nullptr;
  }

  virtual ~Rpc() {}

  error_t Initialize(void) {
    return (Ipc::Initialize(TEMPEST_IPC_NAME, sizeof(IpcData)));
  }

  error_t ServerRegister(pid_t& pid) {
    //
    // Register relay PID in shared memory
    // If realy already running return EEXIST and the pid of the running process
    //  
    error_t err = 0;
    pid = -1;

    if (!(err = Acquire((void*&)shm_))) {
      if (shm_->srv) {
        pid = shm_->srv;
        err = EEXIST;          
      }
      else shm_->srv = getpid();

      error_t rel = Release((void*&)shm_);
      if (!err) err = rel;
    }

    return (err);
  }

  error_t BlockSignals(sigset_t* set = nullptr) {
    //
    // If set = nullptr all signals are blocked
    //
    error_t err = 0;

    sigset_t* ptmp; 
    sigset_t tmp;

    if (set) {
      ptmp = set;
      sigemptyset(ptmp);
      sigaddset(ptmp, SIGINT);
      sigaddset(ptmp, SIGTERM);
      sigaddset(ptmp, SIGUSR1);      
    }    
    else {
      ptmp = &tmp;      
      sigfillset(ptmp);
    }

    if (sigprocmask(SIG_BLOCK, ptmp, nullptr) == -1) err = errno;

    return (err);
  }

  error_t ServerSignals(Relay& relay, const string& version) {
    //
    // Relay signal handler
    //
    error_t err = 0;

    sigset_t set;
    err = BlockSignals(&set);

    int sig;
    while (!err && !(err = sigwait(&set, &sig)) && sig == SIGUSR1) {
      if (!(err = Acquire((void*&)shm_))) {

        switch (shm_->cmd) {
        case Command::STATS:
          CopyStringToShm(relay.Stats());
          shm_->err = 0;
          break;

        case Command::VERSION:
          CopyStringToShm(version);
          shm_->err = 0;
          break;

        default:
          shm_->err = EINVAL;
          break;
        }

        err = kill(shm_->cli, SIGUSR1);

        error_t rel = Release((void*&)shm_);
        if (!err) err = rel;
      }
    }
    
    return (err);
  }

  error_t ClientCommand(Command cmd, pid_t& pid) {
    //
    // Send the relay a command
    // If the relay is not runnning return ENOENT otherwise the PID of the process
    //  
    error_t err = 0;
    pid = -1;

    if (!(err = BlockSignals()) && !(err = Acquire((void*&)shm_))) {
      if (shm_->srv) {
        pid = shm_->srv;
        shm_->cli = getpid();
        shm_->cmd = cmd;
        shm_->err = 0;
        shm_->buffer[0] = '\0';                
      }
      else err = ENOENT;          

      if (!err) err = kill(shm_->srv, (cmd == Command::STOP)? SIGTERM: SIGUSR1);

      error_t rel = Release((void*&)shm_);
      if (!err) err = rel;
    }

    return (err);
  }

  error_t ClientSignals(string& msg) {
    //
    // Client signal handler
    //    
    error_t err = 0;
    msg.clear();

    sigset_t set;
    int sig;
    if (!(err = BlockSignals(&set)) && !(err = sigwait(&set, &sig)) && sig == SIGUSR1 && !(err = Acquire((void*&)shm_))) {
      if (!(err = shm_->err)) {
        switch (shm_->cmd) {
        case Command::STATS:
          msg = shm_->buffer; 
          break;

        case Command::VERSION:
          msg = shm_->buffer; 
          break;

        default:
          err = EINVAL;
          break;
        }
      }

      shm_->cli = 0;
      shm_->cmd = Command::NONE;
      shm_->err = 0;   
      shm_->buffer[0] = '\0';    

      error_t rel = Release((void*&)shm_);
      if (!err) err = rel;
    }
    
    return (err);
  }

private:

  void CopyStringToShm(const string& str) {
    size_t max = min(str.length(), sizeof(shm_->buffer) - 1);
    strncpy(shm_->buffer, str.c_str(), max);
    shm_->buffer[max] = '\0';
  }

  struct IpcData {
    pid_t srv;
    pid_t cli;
    Command cmd;
    error_t err;
    char buffer[2048];  
  }* shm_;

};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_IPC
