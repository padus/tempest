//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: class to handle application version info
//

#ifndef TEMPEST_VERSION
#define TEMPEST_VERSION

#define TEMPEST_VERSION_MAJOR          0
#define TEMPEST_VERSION_MINOR          1
#define TEMPEST_VERSION_BUILD          1
#define TEMPEST_VERSION_LABEL          "alpha"

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Version {
public:
  static int getMajor(void) { return (TEMPEST_VERSION_MAJOR); }
  static int getMinor(void) { return (TEMPEST_VERSION_MINOR); }
  static int getBuild(void) { return (TEMPEST_VERSION_BUILD); }
  static string getLabel(void) { return (TEMPEST_VERSION_LABEL); }
  
  static string getSemantic(void) {
 
    ostringstream semantic;
    
    semantic << "v" << TEMPEST_VERSION_MAJOR << "." << TEMPEST_VERSION_MINOR << "." << TEMPEST_VERSION_BUILD << "-" << TEMPEST_VERSION_LABEL; 

    return (semantic.str());
  }
};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_VERSION
