//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
// 
// Description: main source file
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
// for the specific language governing permissions and limitations under the License.
//

#ifndef TEMPEST_MAIN
#define TEMPEST_MAIN

// Includes -------------------------------------------------------------------------------------------------------------------

#include "log.hpp"
#include "version.hpp"
#include "arguments.hpp"

#include <string>
#include <sstream>
#include <iostream>

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

#endif // TEMPEST_MAIN
