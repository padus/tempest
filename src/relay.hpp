//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: transmit JSON/Ecowitt data to the specified host (Hubitat)
//

#ifndef TEMPEST_RELAY
#define TEMPEST_RELAY

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Relay {
private:
  const string url_;
  const int ecowitt_;
  const int queue_max_;

  condition_variable queue_empty_;
  mutex queue_access_;
  queue<string> queue_;
  bool queue_writer_alive_{true};
  bool queue_reader_alive_{true};

public:
  const string& GetUrl() { return (url_); }
  bool GetEcowitt() { return (ecowitt_ > 0); }
  int GetInterval() { return (ecowitt_); }

  Relay(const string& url, int ecowitt, int queue_max = 128): url_{url}, ecowitt_{ecowitt}, queue_max_{queue_max} {}

  Relay(int ecowitt, int queue_max = 128): url_(""), ecowitt_{ecowitt}, queue_max_{queue_max} {}

  void ReceiverEnded(void) {
    scoped_lock<mutex> lock{queue_access_};

    queue_writer_alive_ = false;

    // if the trasmitter is waiting on an empty queue we wakeup it up since it's not going to get anything new
    queue_empty_.notify_one(); 
  }

  inline bool HasReceiverEnded(void) { return (!queue_writer_alive_); }

  void TrasmitterEnded(void) { 
    scoped_lock<mutex> lock{queue_access_};

    queue_reader_alive_ = false;
  }

  inline bool HasTrasmitterEnded(void) { return (!queue_reader_alive_); }

  bool PushString(const char * str) {
    scoped_lock<mutex> lock{queue_access_};

    // if the trasmitter is dead, there's no point filling the queue
    if (HasTrasmitterEnded()) return (false);

    // if the queue is full we discard the oldest element
    if (queue_.size() == queue_max_) queue_.pop();

    // if the queue was empty we wakeup the transmitter
    if (queue_.size() == 0) queue_empty_.notify_one();

    queue_.emplace(str);

    return (true);
  }

  bool PopString(string& str, int wait_seconds_if_empty = 1) {
    unique_lock<mutex> lock{queue_access_};

    // queue is empty: we quit if...
    if (queue_.size() == 0) {

      // the receiver is dead
      if (HasReceiverEnded()) return (false);

      // the wait ends with a timeout 
      if (queue_empty_.wait_for(lock, chrono::seconds(wait_seconds_if_empty)) == cv_status::timeout) return (false);

      // we have a spurious wakeup
      if (queue_.size() == 0) return (false);
    }

    str = queue_.front();
    queue_.pop();

    return (true);
  }

};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_RELAY
