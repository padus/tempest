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

  enum State {
    ERROR = -1,
    OK = 0,  
    RETRY = 1
  };

  State PushString(const char * str) {
    scoped_lock<mutex> lock{queue_access_};

    // if the trasmitter is dead, there's no point filling the queue
    if (HasTrasmitterEnded()) return (State::ERROR);

    // if the queue is full we discard the oldest element
    if (queue_.size() == queue_max_) queue_.pop();

    // if the queue was empty we wakeup the transmitter
    if (queue_.size() == 0) queue_empty_.notify_one();

    queue_.emplace(str);

    return (State::OK);
  }

  State PopString(string& str, int wait_seconds_if_empty = 1) {
    unique_lock<mutex> lock{queue_access_};

    // queue is empty: we quit if...
    if (queue_.size() == 0) {

      // the receiver is dead
      if (HasReceiverEnded()) return (State::ERROR);

      // the wait ends with a timeout 
      if (queue_empty_.wait_for(lock, chrono::seconds(wait_seconds_if_empty)) == cv_status::timeout) return (State::RETRY);

      // we have a spurious wakeup
      if (queue_.size() == 0) return (State::RETRY);
    }

    str = queue_.front();
    queue_.pop();

    return (State::OK);
  }

  int Receiver() {
    int err = EXIT_SUCCESS;

    try {
      LOG_INFO << "Receiver started.";  

      string text;

      do {



      }
      while (PushString(text.c_str()) != Relay::State::ERROR);

    }
    catch (exception const & ex) {
      err = EXIT_FAILURE;
    }

    ReceiverEnded();

    return (err);
  }

  int Transmitter() {
    int err = EXIT_SUCCESS;

    // Relay& relay = *(Relay*)context; 
      
    try {
      LOG_INFO << "Trasmitter started.";  

      string text;
      State stat;

      while ((stat = PopString(text)) != Relay::State::ERROR) {

        if (stat == Relay::State::OK) cout << text << endl;

      }
    }
    catch (exception const & ex) {
      err = EXIT_FAILURE;
    }

    TrasmitterEnded();

    return (err);
  }

};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*
  //
  // Receiver
  //

  #define RECEIVER_MAX_BUFFER     1024                            // max data to receive
  #define RECEIVER_PORT           50222                           // port we receive data from

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
