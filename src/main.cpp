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

#include "arguments.hpp"
#include "relay.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

#define TEMPEST_VERSION         "v0.1.1-alpha"

using namespace std;
using namespace tempest;

int main(int argc, char* const argv[]) {

  int err = EXIT_SUCCESS;
  ostringstream text;

  string url;
  int ecowitt;
  int log;
  bool daemon;

  // normalize application path and deduce log filename from it 
  namespace fs = std::filesystem;
  fs::path app_name = fs::canonical(argv[0]);
  fs::path log_name = app_name.replace_extension(".log");

  // initialize logger
  nanolog::initialize(nanolog::GuaranteedLogger(), log_name.string(), 8);

  // process command line
  Arguments::PrintCommandLine(argc, argv, text);
  LOG_INFO << "Application started (command line: \"" << text.str() << "\").";

  Arguments args = Arguments(argc, argv);

  if (args.IsCommandLineInvalid()) {
    //
    // invalid comman line
    //    

    // log and print error
    text.str("Invalid command line.");
    LOG_ERROR << text.str();
    cout << text.str() << endl;
    
    // print usage
    args.PrintUsage(text);
    cout << endl << text.str();

    err = EXIT_FAILURE;
  }
  else if (args.IsCommandStart(url, ecowitt, log, daemon, text) || args.IsCommandTrace(ecowitt, log, text)) {
    //
    // start trasmitting or tracing (if url is empty)
    //

    nanolog::set_log_level((nanolog::LogLevel)log);

    if (daemon) {
      //
      // daemonize process 
      // <TBD> @mircolino
      // 
    }

    Relay relay{url, ecowitt};

    future<int> rx = async(launch::async, &Relay::Receiver, &relay);
    future<int> tx = async(launch::async, &Relay::Transmitter, &relay);

    //
    // Main thread should handle signals here
    // <TBD> @mircolino
    //

    int err_rx = rx.get();  
    int err_tx = tx.get();  
    if (err == EXIT_SUCCESS) err = (err_rx != EXIT_SUCCESS)? err_rx: err_tx;
  }
  else if (args.IsCommandStop(text)) {
    //
    // stop process gracefully <TBD> @mircolino
    // <TBD> @mircolino
    //
  }
  else if (args.IsCommandVersion(text)) {
    //
    // version
    //

    cout << TEMPEST_VERSION << endl;
  }
  else if (args.IsCommandHelp(text)) {
    //
    // help
    //    

    args.PrintUsage(text);
    cout << text.str();
  }
  else {
    //
    // we should never be here
    //

    text.str("Unknown error processing command line.");
    LOG_ERROR << text.str();
    cout << text.str() << endl;

    err = EXIT_FAILURE;
  }

  text.str("Application ended");
  if (err) LOG_ERROR << text.str() << " with error " << err << ".";
  else LOG_INFO << text.str() << ".";

  return (err);
}

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------
