//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: listen to and receive UDP data from the WeatherFlow Tempest sensor
//

#ifndef TEMPEST_RECEIVER
#define TEMPEST_RECEIVER

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

#include "relay.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

int main_receiver(void * context) {
  int err = EXIT_SUCCESS;

  Relay& relay = *(Relay*)context; 
    
  try {
    LOG_INFO << "Receiver started.";  

    ostringstream text;
    int idx = 0;

    do {

      if (idx == 20) break;

      text.str("");
      text << "string number: " << idx++;

    }
    while (relay.PushString(text.str().c_str()) != Relay::State::ERROR);

  }
  catch (exception const & ex) {
    err = EXIT_FAILURE;
  }

  relay.ReceiverEnded();

  return (err);
}

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

  const string data1 = "{\"serial_number\":\"ST-00006167\",\"type\":\"light_debug\",\"hub_sn\":\"HB-00022166\",\"ob\":[1600155432,0,0,0,0]}";
  const string data2 = "{\"serial_number\":\"HB-00022166\",\"type\":\"hub_status\",\"firmware_revision\":\"143\",\"uptime\":12391,\"rssi\":-42,\"timestamp\":1600155437,\"reset_flags\":\"PIN,WDG\",\"seq\":1237,\"fs\":[1,0,15675411,524288],\"radio_stats\":[22,1,0,3,56815],\"mqtt_stats\":[1,0]}";



#define RECV_MAX_BUFFER         1024                                       // max data to receive
#define RECV_PORT               50222                                      // port we receive data from

  // create a best-effort datagram socket using UDP
  int sock;                                                                // socket

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



*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_RECEIVER
