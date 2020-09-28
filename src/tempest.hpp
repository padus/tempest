//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: accumulated data
//

#ifndef TEMPEST_DATA
#define TEMPEST_DATA

// Includes -------------------------------------------------------------------------------------------------------------------

#include "system.hpp"

#include "convert.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

// -------------------------------------------------------------

enum PrecipitationType {
  NONE = 0,
  RAIN = 1,
  HAIL = 2
};

// -------------------------------------------------------------

enum RadioStatus {
  OFF = 0,
  ON = 1,
  ACTIVE = 3
};

// -------------------------------------------------------------

class SensorStatus {
public:

  SensorStatus(int status = 0) {
    light_uv_failed =     (status & 0b100000000);
    precip_failed =       (status & 0b010000000);
    wind_failed =         (status & 0b001000000);
    rh_failed =           (status & 0b000100000);
    temperature_failed =  (status & 0b000010000);
    pressure_failed =     (status & 0b000001000);
    lightning_disturber = (status & 0b000000100);
    lightning_noise =     (status & 0b000000010);
    lightning_failed =    (status & 0b000000001);
  }

  bool light_uv_failed        : 1;                              // 0b100000000
  bool precip_failed          : 1;                              // 0b010000000
  bool wind_failed            : 1;                              // 0b001000000
  bool rh_failed              : 1;                              // 0b000100000
  bool temperature_failed     : 1;                              // 0b000010000
  bool pressure_failed        : 1;                              // 0b000001000
  bool lightning_disturber    : 1;                              // 0b000000100
  bool lightning_noise        : 1;                              // 0b000000010
  bool lightning_failed       : 1;                              // 0b000000001
};

// -------------------------------------------------------------

class ResetFlags {
public:

  ResetFlags(const string& list = empty_string) {
    BOR = PIN = POR = SFT = WDG = WWD = LPW = false;

    if (!list.empty()) {
      // split comma separated string
      stringstream flags{list};
      vector<string> result;

      while(flags.good()) {
        string substr;
        getline(flags, substr, ',');
        result.push_back(substr);
      }

      for (int i = 0; i < result.size(); i++) {
        const string& flag = result[i];
             if (flag == "BOR") BOR = true;
        else if (flag == "PIN") PIN = true;
        else if (flag == "POR") POR = true;
        else if (flag == "SFT") SFT = true;
        else if (flag == "WDG") WDG = true;
        else if (flag == "WWD") WWD = true;
        else if (flag == "LPW") LPW = true;
      }
    }
  }

  bool BOR                    : 1;                              // 0b001000000 Brownout reset
  bool PIN                    : 1;                              // 0b000100000 PIN reset
  bool POR                    : 1;                              // 0b000010000 Power reset
  bool SFT                    : 1;                              // 0b000001000 Software reset
  bool WDG                    : 1;                              // 0b000000100 Watchdog reset
  bool WWD                    : 1;                              // 0b000000010 Window watchdog reset
  bool LPW                    : 1;                              // 0b000000001 Low-power reset
};

// -------------------------------------------------------------

class Strike {
public:

  Strike(time_t time = 0, double distance = 0, double energy = 0) {
    time_ = time;
    distance_ = distance;
    energy_ = energy;
  }

  time_t time_;
  double distance_;
  double energy_;
};

// -------------------------------------------------------------

class Wind {
public:

  Wind(time_t time = 0, double speed = 0, double direction = 0) {
    time_ = time;
    speed_ = speed;
    direction_ = direction;
  }

  time_t time_;
  double speed_;
  double direction_;
};

// -------------------------------------------------------------

class Sensor {
public:

  Sensor(const string& id = empty_string): id_{id} {
 
    last_update_ = 0;

    version_ = uptime_ = report_interval_ = rssi_ = 0;

    debug_ = false;

    battery_ = temperature_ = humidity_ = pressure_ = 0;

    illuminance_ = uv_ = solar_radiation_ = 0;

    precipitation_day_accumulated_ = precipitation_accumulated_ = 0;
    precipitation_type_ = PrecipitationType::NONE;

    strike_avg_distance_ =  0;
    strike_count_ = 0;

    wind_lull_ = wind_avg_ = wind_gust_ = wind_direction_ = 0;
    wind_sample_interval_ = 0;

    event_count_rain_ = event_count_strike_ = event_count_wind_ = event_count_observation_ = event_count_status_ = 0;
  }

  const string id_;

  time_t last_update_;

  int version_;
  int uptime_;
  int report_interval_;
  int rssi_;

  SensorStatus status_;
  bool debug_;

  double battery_;
  double temperature_;
  double humidity_;
  double pressure_;

  double illuminance_;
  double uv_;
  double solar_radiation_;

  queue<time_t> precipitation_;
  double precipitation_day_accumulated_;
  double precipitation_accumulated_;
  PrecipitationType precipitation_type_;

  queue<Strike> strike_;
  double strike_avg_distance_;
  int strike_count_;

  queue<Wind> wind_;
  double wind_lull_;
  double wind_avg_;
  double wind_gust_;
  double wind_direction_;
  int wind_sample_interval_;

  uint event_count_rain_;                                       // stats
  uint event_count_strike_;                                     // stats
  uint event_count_wind_;                                       // stats
  uint event_count_observation_;                                // stats
  uint event_count_status_;                                     // stats
};

// -------------------------------------------------------------

class Hub {
public:

  Hub(const string& id = empty_string): id_{id} {

    last_update_ = 0;

    version_ = uptime_ = rssi_ = 0;
    seq_ = fs_[0] =  fs_[1] = fs_[2] = fs_[3] = 0;

    radio_version_ = radio_reboot_count_ = radio_i2c_bus_err_count_ = 0;
    radio_ = RadioStatus::OFF;
    mqtt_[0] = mqtt_[1] = 0;

    event_count_status_ = 0;
  }

  // -----------------------------------------------------------

  Sensor& GetSensor(const string& sensor_id) {
    size_t idx;

    for (idx = 0; idx < sensor_.size(); idx++) {
      if (sensor_[idx].id_ == sensor_id) return (sensor_[idx]);
    }
    
    sensor_.emplace_back(sensor_id);
    return (sensor_[idx]);
  }

  const string id_;

  time_t last_update_;

  int version_;
  int uptime_;
  int rssi_;
  ResetFlags reset_;
  int seq_;
  int fs_[4];

  int radio_version_;
  int radio_reboot_count_;
  int radio_i2c_bus_err_count_;

  RadioStatus radio_;

  int mqtt_[2];

  vector<Sensor> sensor_;

  uint event_count_status_;                                     // stats
};

// -------------------------------------------------------------

class Tempest {
public:

  Tempest(size_t queue_max = 128): start_time_{time(nullptr)}, queue_max_{queue_max} {
    event_count_invalid_ = event_count_unknown_ = event_count_debug_ = 0;
  }

  // -----------------------------------------------------------

  string StatsUdp(void) const {
    size_t hubs, sensors;
    
    ostringstream stats{""};

    double uptime = difftime(time(nullptr), start_time_);
    int days = uptime / 86400;
    uptime -= days * 86400;
    int hours = uptime / 3600;
    uptime -= hours * 3600;
    int minutes = uptime / 60;
    uptime -= minutes * 60;
    int seconds = uptime;

    stats << "Uptime: " << days << "d." << hours << "h." << minutes << "m." << seconds << "s" << endl;
    stats << "Invalid Events: " << event_count_invalid_ << endl;
    stats << "Debug Events: " << event_count_debug_ << endl;
    stats << "Unknown Events: " << event_count_unknown_ << endl;
    hubs = hub_.size();
    stats << "Hubs: " << hubs << endl;
    for (size_t i = 0; i < hubs; i++ ) {
      const Hub& hub = hub_[i];
      stats << "[" << i << "]: " << hub.id_ << " " << hub.version_ << endl;
      stats << "     Status Events: " << hub.event_count_status_ << endl;
      sensors = hub.sensor_.size(); 
      stats << "     Sensors: " << sensors << endl;
      for (size_t i = 0; i < sensors; i++ ) {
        const Sensor& sensor = hub.sensor_[i];
        stats << "     [" << i << "]: " << sensor.id_ << " " << sensor.version_ << endl;
        stats << "          Rain Start Events: " << sensor.event_count_rain_ << endl;
        stats << "          Lightning Strike Events: " << sensor.event_count_strike_ << endl;
        stats << "          Rapid wind Events: " << sensor.event_count_wind_ << endl;
        stats << "          Observation Events: " << sensor.event_count_observation_ << endl;
        stats << "          Status Events: " << sensor.event_count_status_ << endl;
      }
    }

    return (stats.str()); 
  }

  // -----------------------------------------------------------

  size_t WriteUdp(const char udp[], size_t udp_len, bool& notify) {
    //
    // return the number of events/observation written to tempest
    // or -1 if error
    //
    size_t size = -1;
    notify = false;

    string err;
    Json event = Json::parse(udp, err);
    if (event == nullptr) {
      event_count_invalid_++;
      LOG_ERROR << "JSON error: " << err << " parsing: " << udp;
    }
    else {
      const string& type = event["type"].string_value();
      size_t obs, idx = 0;
      size = 0;

           if (type == "evt_precip")              { size = UdpRainStart(event); if (size) notify = true; }
      else if (type == "evt_strike")              { size = UdpLightningStrike(event); if (size) notify = true; } 
      else if (type == "rapid_wind")                size = UdpRapidWind(event);
      else if (type == "obs_air")             do { size += UdpAirObservation(event, idx, obs); } while (++idx < obs);
      else if (type == "obs_sky")             do { size += UdpSkyObservation(event, idx, obs); } while (++idx < obs);
      else if (type == "obs_st")              do { size += UdpTempestObservation(event, idx, obs); } while (++idx < obs);
      else if (type == "device_status")             size = UdpDeviceStatus(event);
      else if (type == "hub_status")                size = UdpHubStatus(event);
      else if (type.find("debug") != string::npos)  event_count_debug_++; 
      else                                        { event_count_unknown_++; LOG_WARN << "Unrecognized UDP event: " << udp; }
    }

    return (size);
  }

  // -----------------------------------------------------------

  bool ReadEcowitt(string& data) {
    //
    // return empty string if error
    //
    ostringstream eco{""};

    eco << "Ecowitt Format";

    data = eco.str();
    return (!data.empty());
  }

  // -----------------------------------------------------------

  bool ReadREST(string& data) {
    //
    // return empty string if error
    //
    ostringstream rest{""};

    rest << "WeatherFlow REST Format";

    data = rest.str();
    return (!data.empty());
  }

private:

  // -----------------------------------------------------------

  Hub& GetHub(const string& hub_id) {
    size_t idx;

    for (idx = 0; idx < hub_.size(); idx++) {
      if (hub_[idx].id_ == hub_id) return (hub_[idx]);
    }
    
    hub_.emplace_back(hub_id);
    return (hub_[idx]);
  }

  // -----------------------------------------------------------

  const string& HubId(const Json& event) const { return (event["hub_sn"].string_value()); }
  const string& SensorId(const Json& event) const { return (event["serial_number"].string_value()); }
  double DeviceVersion(const Json& event) const { return (event["firmware_revision"].is_number()? event["firmware_revision"].number_value(): strtod(event["firmware_revision"].string_value().c_str(), nullptr)); }

  // Rain Start Event ------------------------------------------
  //
  // {
  //   "serial_number":"SK-00008453",
  //   "type":"evt_precip",
  //   "hub_sn":"HB-00000001",
  //   "evt":[
  //     1493322445                                             epoch
  //   ]
  // }
  //
  // "{\"serial_number\":\"SK-00008453\",\"type\":\"evt_precip\",\"hub_sn\":\"HB-00000001\",\"evt\":[1493322445]}"
  //

  size_t UdpRainStart(const Json& event) {
    Sensor& sensor = GetHub(HubId(event)).GetSensor(SensorId(event));
    const Json::array& evt = event["evt"].array_items();

    sensor.last_update_ = evt[0].number_value();
    if (sensor.precipitation_.size() >= queue_max_) sensor.precipitation_.pop();
    sensor.precipitation_.emplace(sensor.last_update_);

    sensor.event_count_rain_++;
    return (1);
  }

  // Lightning Strike Event ------------------------------------
  //
  // {
  //   "serial_number":"AR-00004049",
  //   "type":"evt_strike",
  //   "hub_sn":"HB-00000001",
  //   "evt":[
  //     1493322445,                                            epoch
  //     27,                                                    lightning strike distance (km)
  //     3848                                                   lightning strike energy
  //   ]
  // }
  //
  // "{\"serial_number\":\"AR-00004049\",\"type\":\"evt_strike\",\"hub_sn\":\"HB-00000001\",\"evt\":[1493322445,27,3848]}"
  //

  size_t UdpLightningStrike(const Json& event) {
    Sensor& sensor = GetHub(HubId(event)).GetSensor(SensorId(event));
    const Json::array& evt = event["evt"].array_items();

    sensor.last_update_ = evt[0].number_value();
    if (sensor.strike_.size() >= queue_max_) sensor.strike_.pop();
    sensor.strike_.emplace(sensor.last_update_, evt[1].number_value(), evt[2].number_value());

    sensor.event_count_strike_++;
    return (1);
  }

  // Rapid Wind Event ------------------------------------------
  //
  // {
  //   "serial_number":"SK-00008453",
  //   "type":"rapid_wind",
  //   "hub_sn":"HB-00000001",
  //   "ob":[
  //     1493322445,                                            epoch
  //     2.3,                                                   wind speed (mps)
  //     128                                                    wind direction (deg)
  //   ]
  // }
  //
  // "{\"serial_number\":\"SK-00008453\",\"type\":\"rapid_wind\",\"hub_sn\":\"HB-00000001\",\"ob\":[1493322445,2.3,128]}"
  //

  size_t UdpRapidWind(const Json& event) {
    Sensor& sensor = GetHub(HubId(event)).GetSensor(SensorId(event));
    const Json::array& evt = event["ob"].array_items();

    sensor.last_update_ = evt[0].number_value();
    if (sensor.wind_.size() >= queue_max_) sensor.wind_.pop();
    sensor.wind_.emplace(sensor.last_update_, evt[1].number_value(), evt[2].number_value());

    sensor.event_count_wind_++;
    return (1);
  }

  // Air Observation -------------------------------------------
  //
  // {
  //   "serial_number":"AR-00004049",
  //   "type":"obs_air",
  //   "hub_sn":"HB-00000001",
  //   "obs":[
  //     [
  //       1493164835,                                          epoch
  //       835.0,                                               pressure (MB)
  //       10.0,                                                temperature (C)
  //       45,                                                  humidity (%)
  //       0,                                                   lightning strike count
  //       0,                                                   lightning strike avg distance (km)
  //       3.46,                                                battery (V)
  //       1                                                    report interval (minutes)
  //     ]
  //   ],
  //   "firmware_revision":17
  // }
  //
  // "{\"serial_number\":\"AR-00004049\",\"type\":\"obs_air\",\"hub_sn\":\"HB-00000001\",\"obs\":[[1493164835,835.0,10.0,45,0,0,3.46,1]],\"firmware_revision\":17}"
  //

  size_t UdpAirObservation(const Json& event, size_t idx, size_t& size) {
    // first call with idx = 0

    Sensor& sensor = GetHub(HubId(event)).GetSensor(SensorId(event));

    // we should have a vector of observation
    const Json::array& obs = event["obs"].array_items();
    size = obs.size();
    if (idx >= size) return (0);

    sensor.version_ = DeviceVersion(event);
    const Json::array& evt = obs[idx].array_items();

    sensor.last_update_ = evt[0].number_value();
    sensor.pressure_ = evt[1].number_value();
    sensor.temperature_ = evt[2].number_value();
    sensor.humidity_ = evt[3].number_value();
    sensor.strike_count_ = evt[4].number_value();
    sensor.strike_avg_distance_ = evt[5].number_value();
    sensor.battery_ = evt[6].number_value();
    sensor.report_interval_ = evt[7].number_value();

    sensor.event_count_observation_++;
    return (1);
  }

  // Sky Observation -------------------------------------------
  //
  // {
  //   "serial_number":"SK-00008453",
  //   "type":"obs_sky",
  //   "hub_sn":"HB-00000001",
  //   "obs":[
  //     [
  //       1493321340,                                          epoch
  //       9000,                                                illuminance (Lux)
  //       10,                                                  uv (Index)
  //       0.0,                                                 precipitation accumulated (mm)
  //       2.6,                                                 wind lull (minimum 3 second sample) (m/s)
  //       4.6,                                                 wind avg (average over report interval) (m/s)
  //       7.4,                                                 wind gust (maximum 3 second sample) (m/s)
  //       187,                                                 wind direction (deg)
  //       3.12,                                                battery (V)
  //       1,                                                   report interval (minutes)
  //       130,                                                 solar radiation (W/m^2)
  //       null,                                                local day rain accumulation (mm)
  //       0,                                                   precipitation type	(0 = none, 1 = rain, 2 = hail)
  //       3                                                    wind sample interval (seconds)
  //     ]
  //   ],
  //   "firmware_revision":29
  // }
  //
  // "{\"serial_number\":\"SK-00008453\",\"type\":\"obs_sky\",\"hub_sn\":\"HB-00000001\",\"obs\":[[1493321340,9000,10,0.0,2.6,4.6,7.4,187,3.12,1,130,null,0,3]],\"firmware_revision\":29}"
  //

  size_t UdpSkyObservation(const Json& event, size_t idx, size_t& size) {
    // first call with idx = 0

    Sensor& sensor = GetHub(HubId(event)).GetSensor(SensorId(event));

    // we should have a vector of observation
    const Json::array& obs = event["obs"].array_items();
    size = obs.size();
    if (idx >= size) return (0);

    sensor.version_ = DeviceVersion(event);
    const Json::array& evt = obs[idx].array_items();

    sensor.last_update_ = evt[0].number_value();
    sensor.illuminance_ = evt[1].number_value();
    sensor.uv_ = evt[2].number_value();
    sensor.precipitation_accumulated_ = evt[3].number_value();
    sensor.wind_lull_ = evt[4].number_value();
    sensor.wind_avg_ = evt[5].number_value();
    sensor.wind_gust_ = evt[6].number_value();
    sensor.wind_direction_ = evt[7].number_value();
    sensor.battery_ = evt[8].number_value();
    sensor.report_interval_ = evt[9].number_value();
    sensor.solar_radiation_ = evt[10].number_value();
    sensor.precipitation_day_accumulated_ = evt[11].number_value();
    sensor.precipitation_type_ = (PrecipitationType)evt[12].number_value();
    sensor.wind_sample_interval_ = evt[13].number_value();

    sensor.event_count_observation_++;
    return (1);
  }

  // Tempest Observation ---------------------------------------
  //
  // {
  //   "serial_number":"ST-00000512",
  //   "type":"obs_st",
  //   "hub_sn":"HB-00013030",
  //   "obs":[
  //     [
  //       1588948614,                                          epoch
  //       0.18,                                                wind lull (minimum 3 second sample) (m/s)
  //       0.22,                                                wind avg (average over report interval) (m/s)
  //       0.27,                                                wind gust (maximum 3 second sample) (m/s)
  //       144,                                                 wind direction (deg)
  //       6,                                                   wind sample interval (seconds)
  //       1017.57,                                             pressure (MB)
  //       22.37,                                               temperature (C)
  //       50.26,                                               humidity (%)
  //       328,                                                 illuminance (Lux)
  //       0.03,                                                uv (Index)
  //       3,                                                   solar radiation (W/m^2)
  //       0.000000,                                            precipitation accumulated (mm)
  //       0,                                                   precipitation type	(0 = none, 1 = rain, 2 = hail)
  //       0,                                                   lightning strike avg distance (km)
  //       0,                                                   lightning strike count
  //       2.410,                                               battery (V)
  //       1                                                    report interval (minutes)
  //     ]
  //   ],
  //   "firmware_revision":129
  // }
  //
  // "{\"serial_number\":\"ST-00000512\",\"type\":\"obs_st\",\"hub_sn\":\"HB-00013030\",\"obs\":[[1588948614,0.18,0.22,0.27,144,6,1017.57,22.37,50.26,328,0.03,3,0.000000,0,0,0,2.410,1]],\"firmware_revision\":129}"
  //

  size_t UdpTempestObservation(const Json& event, int idx, size_t& size) {
    // first call with idx = 0

    Sensor& sensor = GetHub(HubId(event)).GetSensor(SensorId(event));

    // we should have a vector of observation
    const Json::array& obs = event["obs"].array_items();
    size = obs.size();
    if (idx >= size) return (0);

    sensor.version_ = DeviceVersion(event);
    const Json::array& evt = obs[idx].array_items();

    sensor.last_update_ = evt[0].number_value();
    sensor.wind_lull_ = evt[1].number_value();
    sensor.wind_avg_ = evt[2].number_value();
    sensor.wind_gust_ = evt[3].number_value();
    sensor.wind_direction_ = evt[4].number_value();
    sensor.wind_sample_interval_ = evt[5].number_value();
    sensor.pressure_ = evt[6].number_value();
    sensor.temperature_ = evt[7].number_value();
    sensor.humidity_ = evt[8].number_value();
    sensor.illuminance_ = evt[9].number_value();
    sensor.uv_ = evt[10].number_value();
    sensor.solar_radiation_ = evt[11].number_value();
    sensor.precipitation_accumulated_ = evt[12].number_value();
    sensor.precipitation_type_ = (PrecipitationType)evt[13].number_value();
    sensor.strike_avg_distance_ = evt[14].number_value();
    sensor.strike_count_ = evt[15].number_value();
    sensor.battery_ = evt[16].number_value();
    sensor.report_interval_ = evt[17].number_value();

    sensor.event_count_observation_++;
    return (1);
  }

  // Device Status ---------------------------------------------
  //
  // {
  //   "serial_number":"AR-00004049",
  //   "type":"device_status",
  //   "hub_sn":"HB-00000001",
  //   "timestamp":1510855923,
  //   "uptime":2189,                                           device uptime (seconds)
  //   "voltage":3.50,                                          battery (V)
  //   "firmware_revision":17,
  //   "rssi":-17,                                              device RSSI (dB)
  //   "hub_rssi":-87,                                          hub RSSI (dB)
  //   "sensor_status":0,                                       sensor status (bitfield)
  //   "debug":0                                                debug state (bool)
  // }
  //
  // "{\"serial_number\":\"AR-00004049\",\"type\":\"device_status\",\"hub_sn\":\"HB-00000001\",\"timestamp\":1510855923,\"uptime\":2189,\"voltage\":3.50,\"firmware_revision\":17,\"rssi\":-17,\"hub_rssi\":-87,\"sensor_status\":0,\"debug\":0}"
  //

  size_t UdpDeviceStatus(const Json& event) {
    Hub& hub = GetHub(HubId(event));

    hub.last_update_ = event["timestamp"].number_value();
    hub.rssi_ = event["hub_rssi"].number_value();

    Sensor& sensor = hub.GetSensor(SensorId(event));

    sensor.version_ = DeviceVersion(event);
    sensor.last_update_ = hub.last_update_;
    sensor.uptime_ = event["uptime"].number_value();
    sensor.battery_ = event["voltage"].number_value();
    sensor.rssi_ = event["rssi"].number_value();
    sensor.status_ = event["sensor_status"].number_value();
    sensor.debug_ = event["debug"].number_value();

    sensor.event_count_status_++;
    return (1);
  }

  // Hub Status ------------------------------------------------
  //
  // {
  //   "serial_number":"HB-00000001",
  //   "type":"hub_status",
  //   "firmware_revision":"35",
  //   "uptime":1670133,                                        hub uptime (seconds)
  //   "rssi":-62,                                              hub RSSI (dB)
  //   "timestamp":1495724691,
  //   "reset_flags":"BOR,PIN,POR",                             see tempest::ResetFlags
  //   "seq":48,                                                undocumented
  //   "fs":[                                                   undocumented
  //     1,
  //     0,
  //     15675411,
  //     524288
  //   ],
  //   "radio_stats":[
  //     2,                                                     version
  //     1,                                                     reboot count
  //     0,                                                     I2C bus error count
  //     3                                                      0b00) Off, 0b01) On, 0b11) Active
  //   ],
  //   "mqtt_stats":[                                           undocumented
  //     1,
  //     0
  //   ]
  // }
  //
  // "{\"serial_number\":\"HB-00000001\",\"type\":\"hub_status\",\"firmware_revision\":\"35\",\"uptime\":1670133,\"rssi\":-62,\"timestamp\":1495724691,\"reset_flags\":\"BOR,PIN,POR\",\"seq\":48,\"fs\":[1,0,15675411,524288],\"radio_stats\":[2,1,0,3],\"mqtt_stats\":[1,0]}"
  //

  size_t UdpHubStatus(const Json& event) {
    Hub& hub = GetHub(SensorId(event));

    hub.version_ = DeviceVersion(event);
    hub.last_update_ = event["timestamp"].number_value();

    hub.uptime_ = event["uptime"].number_value();
    hub.rssi_ = event["rssi"].number_value();

    hub.reset_ = ResetFlags(event["reset_flags"].string_value());
    hub.seq_ = event["seq"].number_value();

    const Json::array& fs = event["fs"].array_items();
    hub.fs_[0] = fs[0].number_value();
    hub.fs_[1] = fs[1].number_value();
    hub.fs_[2] = fs[2].number_value();
    hub.fs_[3] = fs[3].number_value();

    const Json::array& radio = event["radio_stats"].array_items();
    hub.radio_version_ = radio[0].number_value();
    hub.radio_reboot_count_ = radio[1].number_value();
    hub.radio_i2c_bus_err_count_ = radio[2].number_value();
    hub.radio_ = (RadioStatus)radio[3].number_value();

    const Json::array& mqtt = event["mqtt_stats"].array_items();
    hub.mqtt_[0] = mqtt[0].number_value();
    hub.mqtt_[1] = mqtt[1].number_value();

    hub.event_count_status_++;
    return (1);
  }

  const time_t start_time_;
  const size_t queue_max_;

  vector<Hub> hub_;

  uint event_count_invalid_;                                    // stats
  uint event_count_unknown_;                                    // stats
  uint event_count_debug_;                                      // stats
};


} // namespace tempest

// Recycle Bin -----------------------------------------------------------------------------------------------------------------

/*

int TestJson(void) {

  const char * test_json[] = {
    "}\"Invalid JSON\"{",
    "{\"serial_number\":\"ST-000XXXXX\",\"type\":\"light_debug\",\"hub_sn\":\"HB-000XXXXX\",\"ob\":[1600542757,20933,1801,0,0]}",
    "{\"serial_number\":\"SK-00008453\",\"type\":\"evt_precip\",\"hub_sn\":\"HB-00000001\",\"evt\":[1493322445]}",
    "{\"serial_number\":\"AR-00004049\",\"type\":\"evt_strike\",\"hub_sn\":\"HB-00000001\",\"evt\":[1493322445,27,3848]}",
    "{\"serial_number\":\"SK-00008453\",\"type\":\"rapid_wind\",\"hub_sn\":\"HB-00000001\",\"ob\":[1493322445,2.3,128]}",
    "{\"serial_number\":\"AR-00004049\",\"type\":\"obs_air\",\"hub_sn\":\"HB-00000001\",\"obs\":[[1493164835,835.0,10.0,45,0,0,3.46,1]],\"firmware_revision\":17}",
    "{\"serial_number\":\"SK-00008453\",\"type\":\"obs_sky\",\"hub_sn\":\"HB-00000001\",\"obs\":[[1493321340,9000,10,0.0,2.6,4.6,7.4,187,3.12,1,130,null,0,3],[1111111111,8000,20,0.2,2.8,5.6,8.4,197,4.12,2,140,null,0,4]],\"firmware_revision\":29}",
    "{\"serial_number\":\"ST-00000512\",\"type\":\"obs_st\",\"hub_sn\":\"HB-00013030\",\"obs\":[[1588948614,0.18,0.22,0.27,144,6,1017.57,22.37,50.26,328,0.03,3,0.000000,0,0,0,2.410,1]],\"firmware_revision\":129}",
    "{\"serial_number\":\"AR-00004049\",\"type\":\"device_status\",\"hub_sn\":\"HB-00000001\",\"timestamp\":1510855923,\"uptime\":2189,\"voltage\":3.50,\"firmware_revision\":17,\"rssi\":-17,\"hub_rssi\":-87,\"sensor_status\":7,\"debug\":1}",
    "{\"serial_number\":\"HB-00000001\",\"type\":\"hub_status\",\"firmware_revision\":\"35\",\"uptime\":1670133,\"rssi\":-62,\"timestamp\":1495724691,\"reset_flags\":\"BOR,PIN,POR\",\"seq\":48,\"fs\":[1,0,15675411,524288],\"radio_stats\":[2,1,0,3],\"mqtt_stats\":[1,0]}"
  };

  Tempest t;

  UdpEvent id;
  size_t obs;

  for (size_t i = 0; i < (sizeof(test_json) / sizeof(test_json[0])); i++) {

    id = t.ParseUdp(test_json[i], obs);
    cout << "Event: " << id << ",\t Observations: " << obs << endl;
  }

  return (0);
}

*/

// EOF -------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_QUEUE
