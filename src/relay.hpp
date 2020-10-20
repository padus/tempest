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

#include "system.hpp"

#include "log.hpp"
#include "codec.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Relay: Tempest {
public:

  enum Format {
    JSON = 0,
    REST = 1,
    ECOWITT = 2
  };

  Relay(const string& url, Format format, int interval, Log::Facility facility, Log::Level level, int port = 50222, int buffer_max = 1024, int queue_max = 128, int io_timeout = 1):
    Tempest(queue_max), url_{url}, format_{format}, interval_{interval? (interval * 60): 60}, facility_{facility}, level_{level}, port_{port}, buffer_max_{buffer_max}, io_timeout_{io_timeout} {}

  inline void Stop(void) { Exit(); }

  int Receiver() {
    int err = EXIT_SUCCESS;
    int sock = -1;

    bool trace = (url_.empty() && format_ == Format::JSON);

    // Initialize log stream
    Log log{facility_, level_};

    try {
      TLOG_INFO(log) << "Receiver started." << endl;

      // Create a best-effort datagram socket using UDP
      if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        TLOG_ERROR(log) << "socket() failed: " << strerror(errno) << "." << endl;
        throw runtime_error("socket()");
      }

      // Construct bind structure and bind to the broadcast port
      struct sockaddr_in broadcast_addr;                        // broadcast address
      memset(&broadcast_addr, 0, sizeof(broadcast_addr));       // zero out structure
      broadcast_addr.sin_family = AF_INET;                      // internet address family
      broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);       // any incoming interface
      broadcast_addr.sin_port = htons(port_);                   // broadcast port

      if (bind(sock, (const struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr)) == -1) {
        TLOG_ERROR(log) << "bind() failed: " << strerror(errno) << "." << endl;
        throw runtime_error("bind()");
      }

      // Receive a single datagram from the server
      struct sockaddr_in receive_addr;
      socklen_t receive_addr_len = sizeof(receive_addr);
      char receive_buffer[buffer_max_];                         // buffer for received data
      size_t receive_len;                                       // length of received data

      struct timeval receive_to;
      receive_to.tv_sec = io_timeout_;
      receive_to.tv_usec = 0;
      fd_set receive_fds;

      do {
        FD_ZERO(&receive_fds);
        FD_SET(sock, &receive_fds);

        switch (select(sock + 1, &receive_fds, NULL, NULL, &receive_to)) {
        case -1:
          TLOG_ERROR(log) << "select() failed: " << strerror(errno) << "." << endl;
          throw runtime_error("select()");

        case 0:
          // Timeout
          receive_len = 0;
          break;

        default:
          if ((receive_len = recvfrom(sock, receive_buffer, sizeof(receive_buffer) - 1, 0, (struct sockaddr *) &receive_addr, &receive_addr_len)) == -1) {
            TLOG_ERROR(log) << "recvfrom() failed: " << strerror(errno) << "." << endl;
            throw runtime_error("recvfrom()");
          }

          if (receive_len < 0 || receive_len >= sizeof(receive_buffer)) {
            TLOG_ERROR(log) << "recvfrom() returned: " << receive_len << " bytes." << endl;
            throw runtime_error("recvfrom()");
          }
        }

        if (receive_len) {
          // We got data, let's terminate it
          receive_buffer[receive_len] = '\0';

          if (trace) {
            // Trace
            cout << receive_buffer << endl;
          }
          else {
            // Write data to tempest
            Write(log, receive_buffer, receive_len);
          }
        }
      }
      while (Continue());
    }
    catch (exception const & ex) {
      err = EXIT_FAILURE;
    }

    if (sock != -1) close(sock);
    Exit(true);
    TLOG_INFO(log) << "Receiver ended with return code = " << err << "." << endl;

    return (err);
  }

  int Transmitter() {
    int err = EXIT_SUCCESS;

    // Initialize log
    Log log{facility_, level_};

    bool trace = url_.empty() && format_ != Format::JSON;

    vector<string> data;
    size_t event;

    struct curl_slist* slist = nullptr;
    CURL* curl = nullptr;
    CURLcode res;

    try {
      TLOG_INFO(log) << "Trasmitter started." << endl;

      if (!trace) {
        // Initialize CURL library
        res = curl_global_init(CURL_GLOBAL_ALL);
        if (res != CURLE_OK) {
          TLOG_ERROR(log) << "curl_global_init() failed: " << curl_easy_strerror(res) << "." << endl;
          throw runtime_error("curl_global_init()");
        }

        // Add slist string
        slist = curl_slist_append(nullptr, "Content-Type: application/json");
        if (!slist) {
          TLOG_ERROR(log) << "curl_slist_append() returned a NULL pointer." << endl;
          throw runtime_error("curl_slist_append()");
        }

        // Get a curl handle
        curl = curl_easy_init();
        if (!curl) {
          TLOG_ERROR(log) << "curl_easy_init() returned a NULL pointer." << endl;
          throw runtime_error("curl_easy_init()");
        }

        // Verbose
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Set the URL that is about to receive our POST
        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

        // Enable location redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);
        curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

        // Set timeout
        // curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);
      }

      while (Continue()) {

        data.clear();
        event = Read(log, data);
        while (event--) {

          if (trace) {
            // Trace
            cout << data[event] << endl;
          }
          else {
            // Transmit data
            // Specify the POST data and its lenght
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data[event].c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data[event].length());

            // Perform the request, res will get the return code
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
              TLOG_ERROR(log) << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "." << endl;
              throw runtime_error("curl_easy_perform()");
            }
          }
        }
      }
    }
    catch (exception const & ex) {
      err = EXIT_FAILURE;
    }

    if (!trace) {
      // Free the list again
      if (slist) curl_slist_free_all(slist);

      // Cleanup
      if (curl) curl_easy_cleanup(curl);

      curl_global_cleanup();
    }

    Exit();
    TLOG_INFO(log) << "Trasmitter ended with return code = " << err << "." << endl;

    return (err);
  }

  string Stats(void) {
    //
    // Return tempest data structure statistics
    //
    scoped_lock<mutex> lock{tempest_access_};

    return (StatsUdp());
  }

private:

  void Exit(bool notify = false) {

    exit_ = true;

    if (notify) {
      // Wake up the transmitter if he's sleeping
      scoped_lock<mutex> lock{tempest_access_};

      transmitter_.notify_one();
    }
  }

  inline bool Continue(void) { return (!exit_); }

  size_t Write(Log& log, const char data[], size_t data_len) {
    //
    // Return the number of events/observation written to tempest
    // or 0 if error/debug/unrecognized
    //
    scoped_lock<mutex> lock{tempest_access_};

    bool notify = false;

    size_t event = WriteUdp(log, data, data_len, notify);

    // wake up the transmitter if he's sleeping
    if (notify) transmitter_.notify_one();

    return (event);
  }

  size_t Read(Log& log, vector<string>& data) {
    //
    // Return the number of events/observation read from tempest
    // or 0 if error
    //
    unique_lock<mutex> lock{tempest_access_};

    transmitter_.wait_for(lock, chrono::seconds(interval_));
    // == cv_status::timeout

    return ((format_ == Format::REST)? ReadREST(log, data): ReadEcowitt(log, data));
  }

  condition_variable transmitter_;
  mutex tempest_access_;
  atomic<bool> exit_{false};

  const int buffer_max_;
  const int io_timeout_;
  const int port_;
  const string url_;
  const Format format_;
  const int interval_;                                          // in seconds
  const Log::Level level_;
  const Log::Facility facility_;
};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_RELAY
