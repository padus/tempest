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
#include "queue.hpp"
#include "relay.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

#define TEMPEST_VERSION         "v0.1.1-alpha"

using namespace std;
using namespace tempest;

int main(int argc, char* const argv[]) {

  int err = EXIT_SUCCESS;
  string text;

  string url;
  Arguments::DataFormat format;
  int interval;
  nanolog::LogLevel log;
  
  bool daemon = false;

  // normalize application path and deduce log filename from it 
  namespace fs = std::filesystem;
  fs::path app_name = fs::canonical(argv[0]);
  fs::path log_name = app_name.replace_extension(".log");

  // initialize logger
  nanolog::initialize(nanolog::GuaranteedLogger(), log_name.string(), 1);

  // process command line
  Arguments::PrintCommandLine(argc, argv, text);
  LOG_INFO << "Application started (command line: \"" << text << "\").";

  Arguments args = Arguments(argc, argv);

  if (args.IsCommandLineInvalid()) {
    //
    // invalid comman line
    //    

    // log and print error
    text = "Invalid command line.";
    LOG_ERROR << text;
    cout << text << endl;
    
    // print usage
    args.PrintUsage(text);
    cout << endl << text;

    err = EXIT_FAILURE;
  }
  else if (args.IsCommandStart(url, format, interval, log, daemon, text) || args.IsCommandTrace(format, interval, log, text)) {
    //
    // start trasmitting or tracing (if url is empty)
    //

    nanolog::set_log_level(log);

    if (daemon) {
      //
      // daemonize process 
      // <TBD> @mircolino
      // 
    }

    Relay relay{url, format, interval};

    future<int> rx = async(launch::async, &Relay::Receiver, &relay);
    future<int> tx = async(launch::async, &Relay::Transmitter, &relay);

    //
    // Main thread should handle signals here
    // <TBD> @mircolino
    //
    this_thread::sleep_for(chrono::seconds(300));
    relay.Stop();

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
    cout << text;
  }
  else {
    //
    // we should never be here
    //

    text = "Unknown error processing command line.";
    LOG_ERROR << text;
    cout << text << endl;

    err = EXIT_FAILURE;
  }

  LOG_INFO << "Application ended with err = " << err << ".";

  return (err);
}

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------
