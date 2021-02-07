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

Login to your Linux host, either locally or via SSH.

### Firewall

For the relay to properly receive UDP data from the WeatherFlow Tempest station, incoming port UPD 50222 needs to be open.  
If the host running the relay has a firewall and, for example, you are using UFW, you can use the following command:

```text
  ~# sudo ufw allow 50222/udp
```

### Test UDP Relay

Now that UDP port 50222 is open let's test the relay to make sure it's properly receiving UDP data from the weather station.  
Download the relay from github, make it executable and then start it in tracing mode:

```text
  ~# wget https://github.com/mircolino/tempest/raw/master/bin/tempest
  ~# chmod +x tempest
  ~# ./tempest --trace
```

If the realy starts displaying incoming UDP data, it means everytihng is working as expected. Hit \<CTRL>+\<C> to exit the relay and delete the file:

```text
  ~# rm ./tempest
```

### Install the UDP Relay as a system service

For the relay to automatically start every time the host boots up, it need to be installed as a system service.  
Download the relay from github, make it executable:

```text
  ~# sudo wget -P /usr/local/bin https://github.com/mircolino/tempest/raw/master/bin/tempest
  ~# sudo chmod +x /usr/local/bin/tempest
```

Using your favourite editor create the file */etc/systemd/system/tempest.service* with the content below, making sure to update the URL with the correct Hubitat ip address or hostname:

```text
  ~# sudo nano /etc/systemd/system/tempest.service

     [Unit]
     Description=WeatherFlow Tempest UDP Relay

     Wants=network.target
     After=syslog.target network-online.target

     [Service]
     Type=forking
     ExecStart=/usr/local/bin/tempest "--url=http://<hubitat ip>:39501" --daemon
     Restart=on-failure
     RestartSec=60
     KillMode=process

     [Install]
     WantedBy=multi-user.target
```

Now we just need to create and start the service:

```text
  ~# sudo systemctl daemon-reload
  ~# sudo systemctl enable tempest.service
  ~# sudo systemctl start tempest.service
```

If the relay service is running correctly, the next command should display a green "dot" and "active" status:

```text
  ~# sudo systemctl status tempest.service
```

### Update the UDP Relay

To check the current relay version:

```text
  ~# sudo tempest --version
```

To update the relay:

```text
  ~# sudo systemctl stop tempest.service
  ~# sudo rm /usr/local/bin/tempest
  ~# sudo wget -P /usr/local/bin https://github.com/mircolino/tempest/raw/master/bin/tempest
  ~# sudo chmod +x /usr/local/bin/tempest
  ~# sudo systemctl start tempest.service  
```

### Uninstall the UDP Relay

To completely stop and remove the relay from the host:

```text
  ~# sudo systemctl stop tempest.service
  ~# sudo systemctl disable tempest.service
  ~# rm rm /etc/systemd/system/tempest.service
  ~# rm /usr/local/bin/tempest  
```

### UDP Relay Logs and Statistics

To access relay logs:

```text
  ~# sudo grep tempest /var/log/daemon.log
```

To display relay statistics:

```text
  ~# sudo tempest --stats
```

## Relay Command Line Reference

  ```text
  Usage:        tempest [OPTIONS]

  Commands:

  Relay:        tempest --url=<url> [--format=<fmt>] [--interval=<min>]
                        [--log=<lev>] [--daemon]
  Trace:        tempest --trace [--format=<fmt>] [--interval=<min>]
                        [--log=<lev>]
  Stop:         tempest --stop
  Stats:        tempest --stats
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
  -x | --stats          print relay statistics
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
