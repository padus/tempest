//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: application entry point
//
// History:
//
// 2020.09.03:  started development
//              implemented UDP broadcast reading and local tracing
// 2020.09.21:  v0.1.1-alpha
//              build functionally complete
// 2020.10.05:  v1.0.36-beta
//              rewrote algorithm to calculate 10 minutes average wind speed and direction
//              fixed a bug that would incorrectly reset rain event accumulation
//              added IPC
//              added relay statistics
// 2020.10.20:  v1.1.46
//              fixed a bug where the parent would not exit if all workers errored out
//              added receiver and trasmitter retry error
// 2021.02.07:  v1.1.52
//              added SO_REUSEPORT option to listening socket
//              added buinary build platform and cpu directory identifiers 
// 2021.05.05:  v1.1.55
//

// Includes -------------------------------------------------------------------------------------------------------------------

#include <system.hpp>

#include "log.hpp"
#include "args.hpp"
#include "convert.hpp"
#include "ipc.hpp"
#include "codec.hpp"
#include "relay.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

#define TEMPEST_VERSION         "v1.1.55"

using namespace std;
using namespace tempest;

int main(int argc, char* const argv[]) {
  error_t err = 0;

  // Scope IPC so in case of exception it is properly deinitialized
  Rpc ipc;

  // Parse command line
  Arguments args = Arguments(argc, argv);

  Log::Facility facility = args.IsCommandDaemon()? Log::Facility::daemon: Log::Facility::user;
  Log::Level level = args.GetLogLevel();

  // Initialize a log stream
  Log log{facility, level};

  try {
    string text;

    // Print command line
    Arguments::PrintCommandLine(argc, argv, text);
    TLOG_INFO(log) << "Application started (command line: \"" << text << "\")." << endl;

    if (args.IsCommandLineInvalid()) {
      //
      // Invalid comman line
      //

      // Log and print error
      text = "Invalid command line.";
      TLOG_ERROR(log) << text << endl;
      cerr << text << endl;

      // Print usage
      args.PrintUsage(text);
      cout << endl << text;

      throw invalid_argument("command line");
    }

    string url;
    Relay::Format format;
    int interval;

    if (args.IsCommandRelay(url, format, interval, text) || args.IsCommandTrace(format, interval, text)) {
      //
      // Start trasmitting or tracing (if url is empty)
      //
      ostringstream oss;

      if (args.IsCommandDaemon() && (err = daemon(0,0))) {
        oss << "Error demonizing " << argv[0] << ": " << strerror(err) << "." << endl;
        TLOG_ERROR(log) << oss.str();
        cerr << oss.str();
        throw runtime_error("daemon()");
      }

      //
      // Register our PID if we are not already running
      // 
      pid_t pid; 

      if ((err = ipc.Initialize()) || (err = ipc.ServerRegister(pid))) {
        if (err == EEXIST) oss << argv[0] << "(" << pid << ") " << "already running." << endl;
        else oss << "Error registering relay IPC: " << strerror(err) << "." << endl;
        TLOG_ERROR(log) << oss.str();
        cerr << oss.str();
        throw runtime_error("ipc.Server()");
      }

      //
      // Start relay
      // 
      Relay relay{url, format, interval, facility, level};

      // Worker thread should not receive signals
      ipc.BlockSignals();

      future<int> rx = async(launch::async, &Relay::Receiver, &relay);
      future<int> tx = async(launch::async, &Relay::Transmitter, &relay);

      //
      // Handle signals
      //
      if (err = ipc.ServerSignals(relay, TEMPEST_VERSION)) {
        oss << "Error handling IPC: " << strerror(err) << "." << endl;
        TLOG_ERROR(log) << oss.str();
        cerr << oss.str();
      }

      relay.Stop();

      int err_rx = rx.get();
      int err_tx = tx.get();
      if (!err) err = err_rx? err_rx: err_tx;
    }
    else if (args.IsCommandStop(text)) {
      //
      // Stop tempest relay
      //
      pid_t pid; 
      ostringstream oss;

      if ((err = ipc.Initialize()) || (err = ipc.ClientCommand(Rpc::Command::STOP, pid))) {
        if (err == ENOENT) oss << argv[0] << " not running." << endl;
        else oss << "Error stopping " << argv[0] << "(" << pid << "): " << strerror(err) << "." << endl;
        TLOG_ERROR(log) << oss.str();
        cerr << oss.str();
      }
      else {
        oss << argv[0] << "(" << pid << ") stopped." << endl;
        TLOG_INFO(log) << oss.str();
        cout << oss.str();
      }
    }
    else if (args.IsCommandStats(text)) {
      //
      // Print relay statistics
      //  
      pid_t pid;
      ostringstream oss;

      if ((err = ipc.Initialize()) || (err = ipc.ClientCommand(Rpc::Command::STATS, pid))) {      
        if (err == ENOENT) oss << argv[0] << " not running." << endl;
        else oss << "Error getting stats from " << argv[0] << "(" << pid << "): " << strerror(err) << "." << endl;
        TLOG_ERROR(log) << oss.str();
        cerr << oss.str();
      }
      else if (err = ipc.ClientSignals(text)) {
        oss << "Error handling IPC: " << strerror(err) << "." << endl;
        TLOG_ERROR(log) << oss.str();
        cerr << oss.str();
      }
      else {        
        cout << text;
      }
    }
    else if (args.IsCommandVersion(text)) {
      //
      // Version
      //
      pid_t pid;

      cout << TEMPEST_VERSION;
      if (!ipc.Initialize() && !ipc.ClientCommand(Rpc::Command::VERSION, pid) && !ipc.ClientSignals(text)) cout << " (running: " << text << ")";
      cout << endl;
    }
    else if (args.IsCommandHelp(text)) {
      //
      // Help
      //

      args.PrintUsage(text);
      cout << text;
    }
    else {
      //
      // We really should not end up here
      //

      text = "Unknown error processing command line.";
      TLOG_ERROR(log) << text << endl;
      cerr << text << endl;

      err = EXIT_FAILURE;
    }
  }
  catch (exception const & ex) {
    if (!err) err = EXIT_FAILURE;
  }

  TLOG_INFO(log) << "Application ended with return code = " << err << "." << endl;

  return (err);
}

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*


      vector<pid_t> pids = get_pid_from_name(argv[0]);
      int killed = 0;

      for (vector<pid_t>::iterator pid = pids.begin(); pid != pids.end(); ++pid) {
        if (!kill(*pid, SIGTERM)) {
          // Success
          cout << "SIGTERM sent to " << argv[0] << ":" << *pid << "." << endl;
        }
        else {
          // Error
          TLOG_ERROR(log) << "kill() failed: " << strerror(errno) << "." << endl;
          cout << "Error sending SIGTERM to " << argv[0] << ":" << *pid << "." << endl;
        }

        killed++; 
      }

      if (!killed) {
        cout << "Process or daemon not found." << endl;
      }


vector<pid_t> get_pid_from_name(const char* proc_path) {
  //
  // Return a list of process IDs with the same process name (excluding ourselves)
  // Only compatible with Linux
  //
  vector<pid_t> pids;

  // Open the /proc directory
  DIR *dp = opendir("/proc");
  if (dp) {
    // Remove the path (if any) from our own process pathname
    string proc_name = proc_path;
    size_t pos = proc_name.rfind('/');
    if (pos != string::npos) proc_name = proc_name.substr(pos + 1);

    // Enumerate all entries in directory until process found
    struct dirent *dirp;
    pid_t pid;

    while (dirp = readdir(dp)) {
      // Skip non-numeric entries
      if (!(pid = atoi(dirp->d_name))) continue;

      // Read contents of virtual /proc/{pid}/cmdline file
      string cmd_path = string("/proc/") + dirp->d_name + "/cmdline";
      ifstream cmd_file(cmd_path.c_str());

      string cmd_line;
      getline(cmd_file, cmd_line);

      if (!cmd_line.empty()) {
        // Keep first cmdline item which contains the program path
        pos = cmd_line.find('\0');
        if (pos != string::npos) cmd_line = cmd_line.substr(0, pos);

        // Keep program name only, removing the path
        pos = cmd_line.rfind('/');
        if (pos != string::npos) cmd_line = cmd_line.substr(pos + 1);

        // Compare against requested process name and make sure it's not us
        if (proc_name == cmd_line && pid != getpid()) pids.push_back(pid);
      }
    }

    closedir(dp);
  }

  return (pids);
}

*/

// EOF ------------------------------------------------------------------------------------------------------------------------
