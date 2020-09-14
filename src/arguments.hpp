//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: class to handle command line arguments and options
//

#ifndef TEMPEST_ARGUMENTS
#define TEMPEST_ARGUMENTS

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Arguments {
private:

  static const char* usage_[];                                 // see initialization below
  static const struct option option_[];                        // see initialization below 

  string address_;
  string path_;
  int port_;
  int ecowitt_;
  int log_;
  bool daemon_;
  bool terminal_;
  bool stop_;
  bool version_;
  bool help_;

  int opts_;                   // valid options count (-1 if invalid)

  static string Trim(const char* str) {
    //  
    // remove leading "=" and leading and trailing spaces
    //
    if (!str) str = "";
           
    return (regex_replace(str, regex("^[=\\s\\t]+|[\\s\\t]+$"), ""));
  }

  static string ShortOptions(void) {
    //
    // build getopt_long() short options from long options data structure
    //
    string opt = "-";

    for (int idx = 0; option_[idx].name; idx++) {
      opt += option_[idx].val;
      if (option_[idx].has_arg) opt += ':';
    } 

    return (opt);
  }

public:

  static void PrintCommandLine(int argc, char* const argv[], ostringstream& text) {
    //
    // print original command line
    //
    text.str(argv[0]);
    for (int idx = 1; idx < argc; idx++) text << " " << argv[idx];
  }

  static void PrintUsage(ostringstream& text) {
    //
    // print usage based on an array of strings
    //
    text.str("");
    for (int idx = 0; usage_[idx]; idx++) text << usage_[idx] << endl;
  }

  Arguments(int argc, char* const argv[]) {
    //
    // Parse the command line and verify its syntax and semantics validity
    //

    // initialize options to "non-present" state
    address_ = "";
    path_ = "";
    port_ = 0;
    ecowitt_ = 0;
    log_ = 0;
    daemon_ = false;
    terminal_ = false;
    stop_ = false;
    version_ = false;
    help_ = false;

    opts_ = 0;                 // -1 if command line is invalid

    try {
      //
      // check command line syntax
      //
      int value, num;
      string arg, option_short;

      opterr = 0;              // silence getopt_long()
      option_short = ShortOptions();

      while ((value = getopt_long(argc, argv, option_short.c_str(), option_, nullptr)) != -1) {
        arg = Trim(optarg);

        switch (value) {
          case 'a':
            if (arg.empty()) throw invalid_argument(arg);
            address_ = arg;
            break;

          case 'x':
            if (arg.empty()) throw invalid_argument(arg);
            else path_ = arg;
            break;

          case 'p':
            num = stoi(arg);
            if (num < 1 || num > 65353) throw out_of_range(arg);
            port_ = num;
            break;

          case 'e':
            num = arg.empty()? 300: stoi(arg);
            if (num < 60 || num > 3600) throw out_of_range(arg);
            ecowitt_ = num;
            break;

          case 'l':
            num = stoi(arg);
            if (num < 1 || num > 4) throw out_of_range(arg);
            log_ = num;
            break;

          case 'd':
            daemon_ = true;
            break;

          case 't':
            terminal_ = true;
            break;

          case 's':
            stop_ = true;
            break;

          case 'v':
            version_ = true;
            break;

          case 'h':
            help_ = true;
            break;

          default:
            throw invalid_argument(arg);
        }

        // new valid option found
        opts_++;  
      }
    
      //
      // check command line semantics
      //
      if (!address_.empty()) {
        // start command
        num = 1;
        if (!path_.empty()) num++;
        if (port_) num++;
        if (ecowitt_) num++;
        if (log_) num++;
        if (daemon_) num++;
        if (opts_ != num) throw invalid_argument("start");
      }
      else if (terminal_) {
        // trace command
        num = 1;
        if (ecowitt_) num++;
        if (log_) num++;
        if (opts_ != num) throw invalid_argument("trace");
      }
      else if (stop_) {
        // stop command
        if (opts_ != 1) throw invalid_argument("stop");
      }
      else if (version_) {
        // version command
        if (opts_ != 1) throw invalid_argument("version");
      }
      else if (help_) {
        // help command
        if (opts_ != 1) throw invalid_argument("help");
      }
      else {
        // empty command line
        if (opts_) throw invalid_argument("invalid command");
      }
    }
    catch (exception const & ex) {
      // invalid command line
      opts_ = -1;
    }
  }

  bool IsCommandLineInvalid(void) const {
    //
    // Return whether the command line is invalid or not
    //
    return (opts_ < 0);
  }

  bool IsCommandLineEmpty(void) const {
    //
    // Return whether the command line is empty or not
    //
    return (opts_ == 0);
  }

  bool IsCommandStart(string& address, string& path, int& port, int& ecowitt, int& log, bool& daemon, ostringstream& text) const {
    //
    // Return whether the start command was invoked and all its parameters 
    //
    if (IsCommandLineInvalid() || address_.empty()) return (false);
    
    address = address_;
    path = path_.empty()? "/data": path_;
    port = !port_? 39501: port_;
    ecowitt = ecowitt_;
    log = !log_? 2: log_;
    daemon = daemon_;

    text.str("");
    text << "tempest --address=" << address;
    text << " --path=" << path;
    text << " --port=" << port;
    if (ecowitt) text << " --ecowitt=" << ecowitt;
    text << " --log=" << log;
    if (daemon) text << " --daemon";

    return (true);
  }

  bool IsCommandTrace(int& ecowitt, int& log, ostringstream& text) const {
    //
    // Return whether the trace command was invoked and all its parameters 
    //
    if (IsCommandLineInvalid() || !terminal_) return (false);
    
    ecowitt = ecowitt_;
    log = !log_? 2: log_;
    
    text.str("");
    text << "tempest --terminal";
    if (ecowitt) text << " --ecowitt=" << ecowitt;
    text << " --log=" << log;

    return (true);
  }

  bool IsCommandStop(ostringstream& text) const {
    //
    // Return whether the stop command was invoked 
    //
    if (IsCommandLineInvalid() || !stop_) return (false);

    text.str("");
    text << "tempest --stop";

    return (true);
  }

  bool IsCommandVersion(ostringstream& text) const {
    //
    // Return whether the version command was invoked 
    //
    if (IsCommandLineInvalid() || !version_) return (false);

    text.str("");
    text << "tempest --version";

    return (true);
  }

  bool IsCommandHelp(ostringstream& text) const {
    //
    // Return whether the help command was invoked 
    //
    if (IsCommandLineInvalid() || (!help_ && !IsCommandLineEmpty())) return (false);
    
    text.str("");
    text << "tempest [--help]";

    return (true);
  }
};

// Static Initialization ------------------------------------------------------------------------------------------------------

const char* Arguments::usage_[] = {
  "Usage:          tempest [OPTIONS]",
  "",
  "Commands:",
  "",    
  "Start:          tempest --address=<host> [--path=<path>] [--port=<port>]",
  "                        [--ecowitt[=<secs>]] [--log=<level>] [--daemon]",
  "Trace:          tempest --terminal [--ecowitt[=<secs>]] [--log=<level>]",
  "Stop:           tempest --stop",
  "Version:        tempest --version",
  "Help:           tempest [--help]",
  "",
  "Options:",
  "",
  "-a | --address=<host>   host name or IP to relay data to",
  "-x | --path=<path>      host path (default if omitted: /data)",
  "-p | --port=<port>      host port (default if omitted: 39501)",
  "-e | --ecowitt[=<secs>] repackage JSON data into Ecowitt format and",
  "                        relay it at specified intervals (valid range:",
  "                        60 <= secs <= 3600, default if omitted: 300)",
  "-l | --log=<level>      1) errors, warnings, info and debug (everything)",
  "                        2) errors, warnings and info (default if omitted)",
  "                        3) errors and warnings",
  "                        4) only errors",
  "-d | --daemon           run as a service",
  "-t | --terminal         relay data to the terminal standard output",
  "-s | --stop             stop the relay and exit gracefully", 
  "-v | --version          print version information",
  "-h | --help             print this help",
  "",
  "Examples:",
  "",
  "tempest --address=hubitat.local --ecowitt --daemon",
  "tempest -a=192.168.1.100 -x=/json -p=8080",
  "tempest --stop",
  nullptr
};

const struct option Arguments::option_[] = {
  {"address",  required_argument, 0, 'a'},
  {"path",     required_argument, 0, 'x'},
  {"port",     required_argument, 0, 'p'},
  {"ecowitt",  optional_argument, 0, 'e'},
  {"log",      required_argument, 0, 'l'},
  {"daemon",   no_argument,       0, 'd'},   
  {"terminal", no_argument,       0, 't'},    
  {"stop",     no_argument,       0, 's'},
  {"version",  no_argument,       0, 'v'},    
  {"help",     no_argument,       0, 'h'},
  {nullptr,    0,                 0, 0  }
};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_ARGUMENTS
