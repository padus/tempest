//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
// 
// Description: main source file
//

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

#include "log.hpp"
#include "version.hpp"
#include "arguments.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

using namespace std;
using namespace tempest;

int main(int argc, char* const argv[]) {

  string address;
  string path;
  int port;
  int ecowitt;
  int log;
  bool daemon;
  ostringstream text;

  // Arguments::PrintCommandLine(argc, argv, text);
  // cout << endl;
  // cout << text.str();

  Arguments args = Arguments(argc, argv);

  if (args.IsCommandLineInvalid()) {
    cout << "Invalid command line." << endl << endl;
    args.PrintUsage(text);
    cout << text.str();

    return (EXIT_FAILURE);
  }

  if (args.IsCommandStart(address, path, port, ecowitt, log, daemon, text)) {
    cout << text.str() << endl;

    return (EXIT_SUCCESS);
  }

  if (args.IsCommandTrace(ecowitt, log, text)) {
    cout << text.str() << endl;

    return (EXIT_SUCCESS);
  }

  if (args.IsCommandStop(text)) {
    cout << text.str() << endl;

    return (EXIT_SUCCESS);
  }

  if (args.IsCommandVersion(text)) {
    cout << Version::getSemantic() << endl;
    
    return (EXIT_SUCCESS);
  }

  if (args.IsCommandHelp(text)) {
    args.PrintUsage(text);
    cout << text.str();

    return (EXIT_SUCCESS);
  }

  // We should never get here
  return (EXIT_FAILURE);
}

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------
