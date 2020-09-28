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

#include "tempest.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Relay: Tempest {
public:
  // -----------------------------------------------------------

  Relay(const string& url, Arguments::DataFormat format, int interval, int port = 50222, int buffer_max = 1024, int queue_max = 128, int io_timeout = 1):
    Tempest(queue_max), url_{url}, format_{format}, interval_{interval? (interval * 60): 60}, port_{port}, buffer_max_{buffer_max}, io_timeout_{io_timeout} {}

  // -----------------------------------------------------------

  inline void Stop(void) { Exit(); }

  // -----------------------------------------------------------

  int Receiver() {
    int err = EXIT_SUCCESS;
    int sock = -1;

    bool trace = (url_.empty() && format_ == Arguments::DataFormat::JSON);

    try {
      LOG_INFO << "Receiver started.";  

      // create a best-effort datagram socket using UDP
      if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        LOG_ERROR << "socket() failed: " << strerror(errno) << ".";
        throw runtime_error("socket()");
      }

      // construct bind structure and bind to the broadcast port
      struct sockaddr_in broadcast_addr;                        // broadcast address
      memset(&broadcast_addr, 0, sizeof(broadcast_addr));       // zero out structure
      broadcast_addr.sin_family = AF_INET;                      // internet address family
      broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);       // any incoming interface
      broadcast_addr.sin_port = htons(port_);                   // broadcast port

      if (bind(sock, (const struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr)) == -1) {
        LOG_ERROR << "bind() failed: " << strerror(errno) << ".";
        throw runtime_error("bind()");        
      }

      // receive a single datagram from the server
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
          LOG_ERROR << "select() failed: " << strerror(errno) << ".";
          throw runtime_error("select()");                

        case 0:
          // timeout
          receive_len = 0;
          break;

        default:
          if ((receive_len = recvfrom(sock, receive_buffer, sizeof(receive_buffer) - 1, 0, (struct sockaddr *) &receive_addr, &receive_addr_len)) == -1) {
            LOG_ERROR << "recvfrom() failed: " << strerror(errno) << ".";
            throw runtime_error("recvfrom()");        
          }
          
          if (receive_len < 0 || receive_len >= sizeof(receive_buffer)) {
            LOG_ERROR << "recvfrom() returned: " << receive_len << " bytes.";
            throw runtime_error("recvfrom()");                 
          }
        }
        
        if (receive_len) {
          // we got data, let's terminate it
          receive_buffer[receive_len] = '\0';

          if (trace) {
            // trace
            cout << receive_buffer << endl;
          }
          else {
            // write data to tempest
            Write(receive_buffer, receive_len);
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
    LOG_INFO << "Receiver ended with error = " << err << ".";  

    return (err);
  }

  // -----------------------------------------------------------

  int Transmitter() {
    int err = EXIT_SUCCESS;

    bool trace = url_.empty() && format_ != Arguments::DataFormat::JSON;

    try {
      LOG_INFO << "Trasmitter started.";  

      string data;
      while (Continue()) {
        if (Read(data)) {

          if (trace) {
            // trace
            cout << data << endl;
            //cout << Stats();
          }
          else {
            //
            // curl data
            //
          }
        }
      }
    }
    catch (exception const & ex) {
      err = EXIT_FAILURE;
    }

    Exit();
    LOG_INFO << "Trasmitter ended with error = " << err << ".";

    return (err);
  }

private:

  // -----------------------------------------------------------

  void Exit(bool notify = false) { 
    
    exit_ = true;

    if (notify) {
      // wake up the transmitter if he's sleeping
      scoped_lock<mutex> lock{tempest_access_};

      transmitter_.notify_one();
    }
  }

  // -----------------------------------------------------------

  inline bool Continue(void) { return (!exit_); }

  // -----------------------------------------------------------
  
  string Stats(void) {
    //
    // return tempest data structure statistics
    //
    scoped_lock<mutex> lock{tempest_access_};

    return (StatsUdp());
  }

  // -----------------------------------------------------------

  size_t Write(const char data[], size_t data_len) {
    //
    // return the number of events/observation written to tempest
    // or -1 if error
    //
    scoped_lock<mutex> lock{tempest_access_};

    bool notify = false;

    size_t events = WriteUdp(data, data_len, notify); 

    // wake up the transmitter if he's sleeping
    if (notify) transmitter_.notify_one(); 

    return (events);
  }

  // -----------------------------------------------------------

  bool Read(string& data) {
    //
    // return true if something was read
    //
    unique_lock<mutex> lock{tempest_access_};

    transmitter_.wait_for(lock, chrono::seconds(interval_));
    // == cv_status::timeout

    return ((format_ == Arguments::DataFormat::REST)? ReadREST(data): ReadEcowitt(data));
  }

  // -----------------------------------------------------------

  condition_variable transmitter_;
  mutex tempest_access_;
  atomic<bool> exit_{false};

  const int buffer_max_;
  const int io_timeout_;
  const int port_;
  const string url_;
  const Arguments::DataFormat format_;
  const int interval_;                                          // in seconds
};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*
  //
  // Receiver
  //


  // create a best-effort datagram socket using UDP
  int sock;                                                     // socket

  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("socket() failed"); 
    return (EXIT_FAILURE); 
  }

  // set timeout to 60 seconds
  struct timeval timeV;
  timeV.tv_sec = 60;
  timeV.tv_usec = 0;
  
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeV, sizeof(timeV)) < 0) {
    perror("setsockopt() failed");
    close(sock);
    return (EXIT_FAILURE); 
  }

  // construct bind structure and bind to the broadcast port
  struct sockaddr_in broadcastAddr;                                        // broadcast address

  memset(&broadcastAddr, 0, sizeof(broadcastAddr));                        // zero out structure
  broadcastAddr.sin_family = AF_INET;                                      // internet address family
  broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);                       // any incoming interface
  broadcastAddr.sin_port = htons(RECV_PORT);                               // broadcast port

  if (bind(sock, (const struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0) {
    perror("bind() failed");
    close(sock);
    return (EXIT_FAILURE); 
  }

  // receive a single datagram from the server
  struct sockaddr_in receiveAddr;
  socklen_t receiveAddrLen = sizeof(receiveAddr);
  char recvBuffer[RECV_MAX_BUFFER];                                        // buffer for received data
  int recvLen;                                                             // length of received data

  if ((recvLen = recvfrom(sock, recvBuffer, sizeof(recvBuffer) - 1, 0, (struct sockaddr *) &receiveAddr, &receiveAddrLen)) < 0) {
    perror("recvfrom() failed");
    close(sock);
    return (EXIT_FAILURE); 
  }    

  recvBuffer[recvLen] = '\0';
  printf("Received: %s\n", recvBuffer);                                    // print the received string
  
  close(sock);

  const string data1 = "{\"serial_number\":\"ST-00006167\",\"type\":\"light_debug\",\"hub_sn\":\"HB-00022166\",\"ob\":[1600155432,0,0,0,0]}";
  const string data2 = "{\"serial_number\":\"HB-00022166\",\"type\":\"hub_status\",\"firmware_revision\":\"143\",\"uptime\":12391,\"rssi\":-42,\"timestamp\":1600155437,\"reset_flags\":\"PIN,WDG\",\"seq\":1237,\"fs\":[1,0,15675411,524288],\"radio_stats\":[22,1,0,3,56815],\"mqtt_stats\":[1,0]}";

  //
  // Transmitter
  //

  CURL *curl;
  CURLcode res;
 
  struct curl_slist *slist = curl_slist_append(nullptr, "Content-Type: application/json");

  // In windows, this will init the winsock stuff
  curl_global_init(CURL_GLOBAL_ALL);
 
  // get a curl handle
  curl = curl_easy_init();
  if (curl) {
    
    #ifdef TEMPEST_DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    #endif

    // set the URL that is about to receive our POST
    curl_easy_setopt(curl, CURLOPT_URL, relay.GetUrl().c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    // enable location redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);
    curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

    // set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);

    // specify the POST data and its lenght
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, relay.data1.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, relay.data1.length());

    // perform the request, res will get the return code
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
 
    // free the list again
    curl_slist_free_all(slist);

    // cleanup
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_RELAY
