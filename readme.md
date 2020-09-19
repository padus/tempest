# WeatherFlow Tempest UDP Relay

A fast and efficient UDP relay service designed to receive WeatherFlow Tempest locally broadcasted [UDP JSON data](https://weatherflow.github.io/SmartWeather/api/udp/v143/), optionally repackage it into Ecowitt format, and then POST it to Hubitat for consuption.

Inline-style:
![Relay Diagram](https://github.com/mircolino/tempest/raw/master/images/diagram.jpg "Relay Diagram")

## Features

- LAN comunication only, no cloud/weather service needed.
- Entirely written in C++ with no dependencies, external libraries or additional packages to install.
- Small, fast, efficient and highly asynchrounous with non-blocking I/O and mutlithreaded queue.
- Designed to leverage exisitng Hubitat driver infrastucture by emulating WeatherFlow REST API or Ecowitt protols.

## Installation Instructions

Just copy the single tempest executable to your host (PC, VM, RPi etc.) and run it with the appropriate arguments:

```text
Usage:        tempest [OPTIONS]

Commands:

Start:        tempest --url=<url> [--format=<fmt>] [--interval=<min>]
                      [--log=<lev>] [--daemon]
Trace:        tempest --trace [--format=<fmt>] [--interval=<min>]
                      [--log=<lev>]
Stop:         tempest --stop
Version:      tempest --version
Help:         tempest [--help]

Options:

-u | --url=<url>      full URL to relay data to
-f | --format=<fmt>   format to which the UDP data is repackaged:
                      0) JSON untranslated, 1) REST API, 2) Ecowitt
                      (default if omitted: 1)
-i | --interval=<min> interval in minutes at which data is relayed:
                      0 <= min <= 30 (default if omitted: 1)
-l | --log=<lev>      0) only errors
                      1) errors and warnings
                      2) errors, warnings and info (default if omitted)
                      3) errors, warnings, info and debug (everything)
-d | --daemon         run as a service
-t | --trace          relay data to the terminal standard output
-s | --stop           stop the relay and exit gracefully
-v | --version        print version information
-h | --help           print this help

Examples:

tempest --url=http://hubitat.local:39501 --format=2 --interval=5
tempest -u=192.168.1.100:39500 -l=1 -d
tempest --stop
```

***

## Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
