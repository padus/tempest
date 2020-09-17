//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: transmit JSON/Ecowitt data to the specified host (Hubitat)
//

#ifndef TEMPEST_TRANSMITTER
#define TEMPEST_TRANSMITTER

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

#include "relay.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

int main_transmitter(void * context) {
  int err = EXIT_SUCCESS;

  Relay& relay = *(Relay*)context; 
    
  try {
    LOG_INFO << "Trasmitter started.";  

    string text;

    while (true) {

      if (relay.PopString(text)) {
        cout << text << endl;
        continue;
      }
      
      // timeout on empty queue
      if (relay.HasReceiverEnded()) break;
    }
  }
  catch (exception const & ex) {
    err = EXIT_FAILURE;
  }

  relay.TrasmitterEnded();

  return (err);
}

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

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

#endif // TEMPEST_TRANSMITTER
