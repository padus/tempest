//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: transmit JSON/Ecowitt data to the specified host (Hubitat)
//

#ifndef TEMPEST_TRACER
#define TEMPEST_TRACER

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

#include "relay.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

int main_tracer(void * context) {
  int err = EXIT_SUCCESS;

  Relay& relay = *(Relay*)context; 
    
  try {
    LOG_INFO << "Tracer started.";  



  }
  catch (exception const & ex) {
    err = EXIT_FAILURE;
  }

  return (err);
}

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_TRACER
