//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: transmit JSON/Ecowitt data to the specified host (Hubitat)
//

#ifndef TEMPEST_QUEUE
#define TEMPEST_QUEUE

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Queue {
private:
  const int queue_max_;

  condition_variable queue_empty_;
  mutex queue_access_;
  queue<string> queue_;
  bool queue_writer_alive_{true};
  bool queue_reader_alive_{true};

public:
  Queue(int queue_max = 128): queue_max_{queue_max} {}

  void WriterEnded(void) {
    scoped_lock<mutex> lock{queue_access_};

    queue_writer_alive_ = false;

    // if the reader is waiting on an empty queue we wakeup it up since it's not going to get anything new
    queue_empty_.notify_one(); 
  }

  inline bool HasWriterEnded(void) { return (!queue_writer_alive_); }

  void ReaderEnded(void) { 
    scoped_lock<mutex> lock{queue_access_};

    queue_reader_alive_ = false;
  }

  inline bool HasReaderEnded(void) { return (!queue_reader_alive_); }

  enum State {
    EXIT = -1,
    OK = 0,  
    RETRY = 1
  };

  State Push(const char * str) {
    scoped_lock<mutex> lock{queue_access_};

    // if the reader is dead, there's no point filling the queue
    if (HasReaderEnded()) return (State::EXIT);

    // the buffer is empty, probably the result of a timeout 
    if (!str || !str[0]) return (State::RETRY);

    // if the queue is full we discard the oldest element
    if (queue_.size() == queue_max_) queue_.pop();

    // if the queue was empty we wakeup the transmitter
    if (queue_.size() == 0) queue_empty_.notify_one();

    queue_.emplace(str);

    return (State::OK);
  }

  State Pop(string& str, int wait_seconds_if_empty = 1) {
    unique_lock<mutex> lock{queue_access_};

    // queue is empty: we quit if...
    if (queue_.size() == 0) {

      // the writer is dead
      if (HasWriterEnded()) return (State::EXIT);

      // the wait ends with a timeout 
      if (queue_empty_.wait_for(lock, chrono::seconds(wait_seconds_if_empty)) == cv_status::timeout) return (State::RETRY);

      // we have a spurious wakeup
      if (queue_.size() == 0) return (State::RETRY);
    }

    str = queue_.front();
    queue_.pop();

    return (State::OK);
  }

};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_QUEUE
