# WeatherFlow Tempest UDP Relay

A fast and efficient UDP relay service designed to receive WeatherFlow Tempest locally broadcasted [UDP JSON data](https://weatherflow.github.io/SmartWeather/api/udp/v143/), optionally repackage it into the Ecowitt format, and then POST it to Hubitat for consumption.

![Diagram](https://github.com/mircolino/tempest/raw/master/images/diagram.jpg "Relay Diagram")

## Features

- LAN communication only, no cloud/weather service needed.
- Entirely written in C++ with no dependencies, external libraries or additional packages to install.
- Small, fast, efficient and highly asynchronous with non-blocking I/O and multithreaded queue.
- Designed to leverage existing Hubitat driver infrastructure by emulating WeatherFlow REST API or Ecowitt protocols.
- Support for multiple Tempest stations

## Installation Instructions

- Copy the single ```tempest``` executable to your host (PC, VM, RPi etc.)
- Make sure the file is executable. If you are using Linux run the following command:

  ```text
  chmod +x tempest
  ```

- If the host running the relay has a firewall, don't forget to open incoming port UPD 50222. If, for example, you are using UFW, you can use the following command:

  ```text
  sudo ufw allow 50222/udp
  ```

- Run the executable with the appropriate arguments:

  ```text
  Usage:        tempest [OPTIONS]

  Commands:

  Relay:        tempest --url=<url> [--format=<fmt>] [--interval=<min>]
                        [--log=<lev>] [--daemon]
  Trace:        tempest --trace [--format=<fmt>] [--interval=<min>]
                        [--log=<lev>]
  Stop:         tempest --stop
  Version:      tempest --version
  Help:         tempest [--help]

  Options:

  -u | --url=<url>      full URL to relay data to
  -f | --format=<fmt>   format to which the UDP data is repackaged:
                        1) REST API, 2) Ecowitt (default if omitted: 2)
  -i | --interval=<min> interval in minutes at which data is relayed:
                        1 <= min <= 30 (default if omitted: 5)
  -l | --log=<lev>      1) only errors
                        2) errors and warnings
                        3) errors, warnings and info (default if omitted)
                        4) errors, warnings, info and debug (everything)
  -d | --daemon         run as a background daemon
  -t | --trace          relay data to the terminal standard output
                        (if both --format and --interval are omitted
                        the source UDP JSON will be traced instead)
  -s | --stop           stop relaying/tracing and exit gracefully
  -v | --version        print version information
  -h | --help           print this help

  Examples:

  tempest --url=http://hubitat.local:39501 --format=2 --interval=5 --daemon
  tempest -u=192.168.1.100:39500 -l=2 -d
  tempest --stop
  ```

## Note to Developers

This application has been developed on a Windows 10 system with Visual Studio Code connected to a WSL2 instance running vanilla Debian 10.5 with the following development packages installed:

  ```text
  sudo apt install build-essential gdb git libcurl4-openssl-dev
  ```

To build your own executable from source, clone the repository and run one of the following:

  ```text
  make release
  ```

or

  ```text
  make debug
  ```

***

## Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
