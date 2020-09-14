//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
// 
// Description: application entry point
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

  // normalize application path and deduce log filename from it 
  namespace fs = std::filesystem;
  fs::path app_name = fs::canonical(argv[0]);
  fs::path log_name = app_name.replace_extension(".log");

  // initialize logger
  nanolog::initialize(nanolog::GuaranteedLogger(), log_name.string(), 8);

  // process command line
  string address;
  string path;
  int port;
  int ecowitt;
  int log;
  bool daemon;

  ostringstream text;

  Arguments args = Arguments(argc, argv);

  if (args.IsCommandLineInvalid()) {
    // print error (to standard output)
    cout << "Invalid command line." << endl << endl;
    args.PrintUsage(text);
    cout << text.str();

    // log error
    Arguments::PrintCommandLine(argc, argv, text);
    LOG_ERROR << "Command line \"" << text.str() << "\" is invalid.";

    return (EXIT_FAILURE);
  }

  if (args.IsCommandStart(address, path, port, ecowitt, log, daemon, text)) {
    nanolog::set_log_level((nanolog::LogLevel)log);

    //
    // start
    //

    return (EXIT_SUCCESS);
  }

  if (args.IsCommandTrace(ecowitt, log, text)) {
    nanolog::set_log_level((nanolog::LogLevel)log);   

    //
    // trace
    //

    return (EXIT_SUCCESS);
  }

  if (args.IsCommandStop(text)) {

    //
    // stop
    //

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
