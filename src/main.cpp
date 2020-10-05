//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: application entry point
//

// Includes -------------------------------------------------------------------------------------------------------------------

#include <system.hpp>

#include "log.hpp"
#include "args.hpp"
#include "convert.hpp"
#include "tempest.hpp"
#include "relay.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

#define TEMPEST_VERSION         "v1.0.37-beta"

using namespace std;
using namespace tempest;

pid_t get_pid_from_name(const char* proc_path) {
  //
  // Return the process ID from the process name, or -1 if it couldn't find it
  // Only Linux compatible
  //
  pid_t pid = -1;

  // Open the /proc directory
  DIR *dp = opendir("/proc");
  if (dp) {
    // Enumerate all entries in directory until process found
    struct dirent *dirp;

    while (pid < 0 && (dirp = readdir(dp))) {
      // Skip non-numeric entries
      pid_t id = atoi(dirp->d_name);

      if (id > 0) {
        // Read contents of virtual /proc/{pid}/cmdline file
        string cmd_path = string("/proc/") + dirp->d_name + "/cmdline";
        ifstream cmd_file(cmd_path.c_str());

        string cmd_line;
        getline(cmd_file, cmd_line);

        if (!cmd_line.empty()) {
          // Keep first cmdline item which contains the program path
          size_t pos = cmd_line.find('\0');
          if (pos != string::npos) cmd_line = cmd_line.substr(0, pos);

          // Keep program name only, removing the path
          pos = cmd_line.rfind('/');
          if (pos != string::npos) cmd_line = cmd_line.substr(pos + 1);

          string proc_name = proc_path;
          pos = proc_name.rfind('/');
          if (pos != string::npos) proc_name = proc_name.substr(pos + 1);

          // Compare against requested process name
          if (proc_name == cmd_line) pid = id;
        }
      }
    }

    closedir(dp);
  }

  return (pid);
}

int main(int argc, char* const argv[]) {

  int err = EXIT_SUCCESS;

  // Parse command line
  Arguments args = Arguments(argc, argv);

  Log::Facility facility = args.IsCommandDaemon()? Log::Facility::daemon: Log::Facility::user;
  Log::Level level = args.GetLogLevel();

  // Initialize a log stream
  Log log{facility, level};

  try {
    string text;

    // Print command line
    Arguments::PrintCommandLine(argc, argv, text);
    TLOG_INFO(log) << "Application started (command line: \"" << text << "\")." << endl;

    if (args.IsCommandLineInvalid()) {
      //
      // Invalid comman line
      //

      // Log and print error
      text = "Invalid command line.";
      TLOG_ERROR(log) << text << endl;
      cout << text << endl;

      // Print usage
      args.PrintUsage(text);
      cout << endl << text;

      throw invalid_argument("command line");
    }

    string url;
    Relay::Format format;
    int interval;

    if (args.IsCommandRelay(url, format, interval, text) || args.IsCommandTrace(format, interval, text)) {
      //
      // Start trasmitting or tracing (if url is empty)
      //

      if (args.IsCommandDaemon() && daemon(0,0) != EXIT_SUCCESS) {
        TLOG_ERROR(log) << "Error demonizing the process." << endl;
        throw runtime_error("daemon()");
      }

      Relay relay{url, format, interval, facility, level};

      future<int> rx = async(launch::async, &Relay::Receiver, &relay);
      future<int> tx = async(launch::async, &Relay::Transmitter, &relay);

      //
      // Handle terminate signals here
      //
      sigset_t set;
      sigemptyset(&set);
      sigaddset(&set, SIGINT);
      sigaddset(&set, SIGTERM);
      sigprocmask(SIG_BLOCK, &set, nullptr);

      int sig;
      err = sigwait(&set, &sig);

      relay.Stop();

      int err_rx = rx.get();
      int err_tx = tx.get();
      if (err == EXIT_SUCCESS) err = (err_rx != EXIT_SUCCESS)? err_rx: err_tx;
    }
    else if (args.IsCommandStop(text)) {
      //
      // Stop another running instance of tempest
      //
      pid_t pid = get_pid_from_name(argv[0]);

      if (pid != -1 && pid != getpid()) {
        // We found a process/daemon and it's not us: send a SIGTERM
        if (!kill(pid, SIGTERM)) {
          // Success
          cout << "SIGTERM sent to " << argv[0] << "." << endl;
        }
        else {
          // Error
          TLOG_ERROR(log) << "kill() failed: " << strerror(errno) << "." << endl;
          cout << "Error sending SIGTERM to " << argv[0] << "." << endl;
        }
      }
      else {
        cout << "Process or daemon not found." << endl;
      }
    }
    else if (args.IsCommandVersion(text)) {
      //
      // Version
      //

      cout << TEMPEST_VERSION << endl;
    }
    else if (args.IsCommandHelp(text)) {
      //
      // Help
      //

      args.PrintUsage(text);
      cout << text;
    }
    else {
      //
      // We really should not end up here
      //

      text = "Unknown error processing command line.";
      TLOG_ERROR(log) << text << endl;
      cout << text << endl;

      err = EXIT_FAILURE;
    }
  }
  catch (exception const & ex) {
    err = EXIT_FAILURE;
  }

  TLOG_INFO(log) << "Application ended with return code = " << err << "." << endl;

  return (err);
}

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------
