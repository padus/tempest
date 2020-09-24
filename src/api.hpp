//
// App:         WeatherFlow Tempest UDP Relay 
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: transmit JSON/Ecowitt data to the specified host (Hubitat)
//

#ifndef TEMPEST_API
#define TEMPEST_API

// Includes -------------------------------------------------------------------------------------------------------------------

#ifndef TEMPEST_SYSTEM
#include <system.hpp>
#endif

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;
using namespace json;

class UdpEvent {
public:

  enum Event {
    evt_precip = 1,             // Rain Start
    evt_strike = 2,             // Lightning Strike
    rapid_wind = 3,             // Rapid Wind
    obs_air = 4,                // Air Observation
    obs_sky = 5,                // Sky Observation
    obs_st = 6,                 // Tempest Observation
    device_status = 7,          // Device Status
    hub_status = 8              // Hub Status
  };

  enum PrecipitationType {
    NONE = 0,
    RAIN = 1,
    HAIL = 2
  };

  enum RadioStatus {
    OFF = 0,
    ON = 1,
    ACTIVE = 3
  };

protected:

  // to prevent from being created directly
  UdpEvent() {}

  bool Init(Event event, double epoch, const char hub_id[], double hub_version, const char device_id[], double device_version) {
    event_ = event;
    epoch_ = epoch;
    strncpy(hub_id_, hub_id, sizeof(hub_id_) - 1);
    hub_id_[sizeof(hub_id_) - 1] = '\0'; 
    hub_version_ = hub_version;
    strncpy(device_id_, device_id, sizeof(device_id_) - 1);
    hub_id_[sizeof(device_id_) - 1] = '\0'; 
    device_version_ = device_version;

    return (true);
  }

  const string & hub_sn(const Json& event) const { return (event["hub_sn"].string_value()); }
  const string & serial_number(const Json& event) const { return (event["serial_number"].string_value()); }
  double firmware_revision(const Json& event) const { return (event["firmware_revision"].is_number()? event["firmware_revision"].number_value(): strtod(event["firmware_revision"].string_value().c_str(), nullptr)); }

private:

  Event event_;
  time_t epoch_;  
  char hub_id_[16];
  int hub_version_;  
  char device_id_[16];
  int device_version_;
};

// Rain Start Event --------------------------------------------
//
// {
//   "serial_number":"SK-00008453",
//   "type":"evt_precip",
//   "hub_sn":"HB-00000001",
//   "evt":[
//     1493322445                                               epoch
//   ]
// }
//
// "{\"serial_number\":\"SK-00008453\",\"type\":\"evt_precip\",\"hub_sn\":\"HB-00000001\",\"evt\":[1493322445]}"
//

class UdpRainStart: UdpEvent {
public:

  UdpRainStart(const Json& event, bool& ok) {
    ok = false;
    memset(this, 0, sizeof(*this));

    const string& device_id = serial_number(event);   
    const string& hub_id = hub_sn(event);
    const Json::array& evt = event["evt"].array_items();

    double epoch = evt[0].number_value();

    if (!device_id.empty() && !hub_id.empty() && epoch) {
      ok = Init(evt_precip, epoch, hub_id.c_str(), 0, device_id.c_str(), 0);
    }
  }

private:

};

// Lightning Strike Event --------------------------------------
//
// {
//   "serial_number":"AR-00004049",
//   "type":"evt_strike",
//   "hub_sn":"HB-00000001",
//   "evt":[
//     1493322445,                                              epoch
//     27,                                                      lightning strike distance (km)
//     3848                                                     lightning strike energy
//   ]
// }
//
// "{\"serial_number\":\"AR-00004049\",\"type\":\"evt_strike\",\"hub_sn\":\"HB-00000001\",\"evt\":[1493322445,27,3848]}"
//

class UdpLightningStrike: UdpEvent {
public:

  UdpLightningStrike(const Json& event, bool& ok) {
    ok = false;
    memset(this, 0, sizeof(*this));

    const string& device_id = serial_number(event);   
    const string& hub_id = hub_sn(event);
    const Json::array& evt = event["evt"].array_items();

    double epoch = evt[0].number_value();

    if (!device_id.empty() && !hub_id.empty() && epoch) {
      strike_distance_ = evt[1].number_value();
      strike_energy_ = evt[2].number_value();

      ok = Init(evt_strike, epoch, hub_id.c_str(), 0, device_id.c_str(), 0);
    }
  }

  UdpLightningStrike() {
    size_t size = sizeof(*this);
    memset(this, 0, size);  
  }

private:

  double strike_distance_;
  double strike_energy_;
};

// Rapid Wind Event --------------------------------------------
//
// {
//   "serial_number":"SK-00008453",
//   "type":"rapid_wind",
//   "hub_sn":"HB-00000001",
//   "ob":[
//     1493322445,                                              epoch
//     2.3,                                                     wind speed (mps)
//     128                                                      wind direction (deg)
//   ]
// }
//
// "{\"serial_number\":\"SK-00008453\",\"type\":\"rapid_wind\",\"hub_sn\":\"HB-00000001\",\"ob\":[1493322445,2.3,128]}"
//

class UdpRapidWind: UdpEvent {
public:

  UdpRapidWind(const Json& event, bool& ok) {
    ok = false;
    memset(this, 0, sizeof(*this));

    const string& device_id = serial_number(event);   
    const string& hub_id = hub_sn(event);
    const Json::array& evt = event["ob"].array_items();

    double epoch = evt[0].number_value();

    if (!device_id.empty() && !hub_id.empty() && epoch) {
      wind_speed_ = evt[1].number_value();
      wind_direction_ = evt[2].number_value();

      ok = Init(rapid_wind, epoch, hub_id.c_str(), 0, device_id.c_str(), 0);
    }
  }

private:

  double wind_speed_;
  double wind_direction_;
};

// Air Observation ---------------------------------------------
//
// {
//   "serial_number":"AR-00004049",
//   "type":"obs_air",
//   "hub_sn":"HB-00000001",
//   "obs":[
//     [
//       1493164835,                                            epoch
//       835.0,                                                 pressure (MB)
//       10.0,                                                  temperature (C)
//       45,                                                    humidity (%)
//       0,                                                     lightning strike count
//       0,                                                     lightning strike avg distance (km)
//       3.46,                                                  battery (V)
//       1                                                      report interval (minutes)
//     ]
//   ],
//   "firmware_revision":17
// }
//
// "{\"serial_number\":\"AR-00004049\",\"type\":\"obs_air\",\"hub_sn\":\"HB-00000001\",\"obs\":[[1493164835,835.0,10.0,45,0,0,3.46,1]],\"firmware_revision\":17}"
// 

class UdpAirObservation: UdpEvent {
public:

  UdpAirObservation(const Json& event, int idx, size_t& size, bool& ok) {
    // first call with idx = 0
    ok = false;
    memset(this, 0, sizeof(*this));

    // we should have a vector of observation
    const Json::array& obs = event["obs"].array_items();
    size = obs.size();
    if (size > 0 && idx < size) {
      const string& device_id = serial_number(event);   
      const string& hub_id = hub_sn(event);
      double device_version = firmware_revision(event);
      const Json::array& evt = obs[idx].array_items();

      double epoch = evt[0].number_value();

      if (!device_id.empty() && !hub_id.empty() && epoch) {
        pressure_ = evt[1].number_value();
        temperature_ = evt[2].number_value();
        humidity_ = evt[3].number_value();
        strike_count_ = evt[4].number_value();
        strike_avg_distance_ = evt[5].number_value();                        
        battery_ = evt[6].number_value();
        report_interval_ = evt[7].number_value();

        ok = Init(obs_air, epoch, hub_id.c_str(), 0, device_id.c_str(), device_version);
      }
    }
  }

private:

  double pressure_;
  double temperature_;
  double humidity_;
  int strike_count_;
  double strike_avg_distance_;
  double battery_;
  int report_interval_;
};

// Sky Observation ---------------------------------------------
//
// {
//   "serial_number":"SK-00008453",
//   "type":"obs_sky",
//   "hub_sn":"HB-00000001",
//   "obs":[
//     [
//       1493321340,                                            epoch
//       9000,                                                  illuminance (Lux)
//       10,                                                    uv (Index)
//       0.0,                                                   precipitation accumulated (mm)
//       2.6,                                                   wind lull (minimum 3 second sample) (m/s)
//       4.6,                                                   wind avg (average over report interval) (m/s)
//       7.4,                                                   wind gust (maximum 3 second sample) (m/s)
//       187,                                                   wind direction (deg)
//       3.12,                                                  battery (V)
//       1,                                                     report interval (minutes)
//       130,                                                   solar radiation (W/m^2)
//       null,                                                  local day rain accumulation (mm)
//       0,                                                     precipitation type	(0 = none, 1 = rain, 2 = hail)
//       3                                                      wind sample interval (seconds)
//     ]
//   ],
//   "firmware_revision":29
// }
//
// "{\"serial_number\":\"SK-00008453\",\"type\":\"obs_sky\",\"hub_sn\":\"HB-00000001\",\"obs\":[[1493321340,9000,10,0.0,2.6,4.6,7.4,187,3.12,1,130,null,0,3]],\"firmware_revision\":29}"
//

class UdpSkyObservation: UdpEvent {
public:

  UdpSkyObservation(const Json& event, int idx, size_t& size, bool& ok) {
    // first call with idx = 0
    ok = false;
    memset(this, 0, sizeof(*this));

    // we should have a vector of observation
    const Json::array& obs = event["obs"].array_items();
    size = obs.size();
    if (size > 0 && idx < size) {
      const string& device_id = serial_number(event);   
      const string& hub_id = hub_sn(event);
      double device_version = firmware_revision(event);
      const Json::array& evt = obs[idx].array_items();

      double epoch = evt[0].number_value();

      if (!device_id.empty() && !hub_id.empty() && epoch) {
        illuminance_ = evt[1].number_value();
        uv_ = evt[2].number_value();
        precipitation_accumulated_ = evt[3].number_value();
        wind_lull_ = evt[4].number_value();
        wind_avg_ = evt[5].number_value();     
        wind_gust_ = evt[6].number_value();
        wind_direction_ = evt[7].number_value();
        battery_ = evt[8].number_value();
        report_interval_ = evt[9].number_value();
        solar_radiation_ = evt[10].number_value();
        local_day_rain_accumulated_ = evt[11].number_value();
        precipitation_type_ = (PrecipitationType)evt[12].number_value();    
        wind_sample_interval_ = evt[13].number_value();

        ok = Init(obs_sky, epoch, hub_id.c_str(), 0, device_id.c_str(), device_version);
      }
    }
  }

private:

  double illuminance_;
  double uv_;
  double precipitation_accumulated_;
  double wind_lull_;
  double wind_avg_;
  double wind_gust_;
  double wind_direction_;
  double battery_;
  int report_interval_;
  double solar_radiation_;
  double local_day_rain_accumulated_;
  PrecipitationType precipitation_type_;
  int wind_sample_interval_;
};

// Tempest Observation -----------------------------------------
// 
// {
//   "serial_number":"ST-00000512",
//   "type":"obs_st",
//   "hub_sn":"HB-00013030",
//   "obs":[
//     [
//       1588948614,                                            epoch
//       0.18,                                                  wind lull (minimum 3 second sample) (m/s)
//       0.22,                                                  wind avg (average over report interval) (m/s)
//       0.27,                                                  wind gust (maximum 3 second sample) (m/s)
//       144,                                                   wind direction (deg)
//       6,                                                     wind sample interval (seconds)
//       1017.57,                                               pressure (MB)
//       22.37,                                                 temperature (C)
//       50.26,                                                 humidity (%)
//       328,                                                   illuminance (Lux)
//       0.03,                                                  uv (Index)
//       3,                                                     solar radiation (W/m^2)
//       0.000000,                                              precipitation accumulated (mm)
//       0,                                                     precipitation type	(0 = none, 1 = rain, 2 = hail)
//       0,                                                     lightning strike avg distance (km)
//       0,                                                     lightning strike count
//       2.410,                                                 battery (V)
//       1                                                      report interval (minutes)
//     ]
//   ],
//   "firmware_revision":129
// }
//
// "{\"serial_number\":\"ST-00000512\",\"type\":\"obs_st\",\"hub_sn\":\"HB-00013030\",\"obs\":[[1588948614,0.18,0.22,0.27,144,6,1017.57,22.37,50.26,328,0.03,3,0.000000,0,0,0,2.410,1]],\"firmware_revision\":129}"
//

class UdpTempestObservation: UdpEvent {
public:

  UdpTempestObservation(const Json& event, int idx, size_t& size, bool& ok) {
    // first call with idx = 0
    ok = false;
    memset(this, 0, sizeof(*this));

    // we should have a vector of observation
    const Json::array& obs = event["obs"].array_items();    
    size = obs.size();
    if (size > 0 && idx < size) {
      const string& device_id = serial_number(event);   
      const string& hub_id = hub_sn(event);
      double device_version = firmware_revision(event);
      const Json::array& evt = obs[idx].array_items();

      double epoch = evt[0].number_value();

      if (!device_id.empty() && !hub_id.empty() && epoch) {
        wind_lull_ = evt[1].number_value();
        wind_avg_ = evt[2].number_value();     
        wind_gust_ = evt[3].number_value();
        wind_direction_ = evt[4].number_value();
        wind_sample_interval_ = evt[5].number_value();
        pressure_ = evt[6].number_value();
        temperature_ = evt[7].number_value(); 
        humidity_ = evt[8].number_value(); 
        illuminance_ = evt[9].number_value();
        uv_ = evt[10].number_value();
        solar_radiation_ = evt[11].number_value();
        precipitation_accumulated_ = evt[12].number_value();
        precipitation_type_ = (PrecipitationType)evt[13].number_value();    
        strike_avg_distance_ = evt[14].number_value();       
        strike_count_ = evt[15].number_value();
        battery_ = evt[16].number_value();
        report_interval_ = evt[17].number_value();

        ok = Init(obs_st, epoch, hub_id.c_str(), 0, device_id.c_str(), device_version);
      }
    }
  }

private:

  double wind_lull_;
  double wind_avg_;
  double wind_gust_;
  double wind_direction_;
  int wind_sample_interval_;
  double pressure_;
  double temperature_;
  double humidity_;
  double illuminance_;
  double uv_;
  double solar_radiation_;
  double precipitation_accumulated_;
  PrecipitationType precipitation_type_;
  double strike_avg_distance_;
  int strike_count_;
  double battery_;
  int report_interval_;
};

// Device Status -----------------------------------------------
//
// {
//   "serial_number":"AR-00004049",
//   "type":"device_status",
//   "hub_sn":"HB-00000001",
//   "timestamp":1510855923,
//   "uptime":2189,                                             device uptime (seconds)
//   "voltage":3.50,                                            battery (V)
//   "firmware_revision":17,
//   "rssi":-17,                                                device RSSI (dB)
//   "hub_rssi":-87,                                            hub RSSI (dB)
//   "sensor_status":0,                                         sensor status (bitfield)
//   "debug":0                                                  debug state (bool)
// }
//
// "{\"serial_number\":\"AR-00004049\",\"type\":\"device_status\",\"hub_sn\":\"HB-00000001\",\"timestamp\":1510855923,\"uptime\":2189,\"voltage\":3.50,\"firmware_revision\":17,\"rssi\":-17,\"hub_rssi\":-87,\"sensor_status\":0,\"debug\":0}"
//

class UdpDeviceStatus: UdpEvent {
public:

  class SensorStatus {
  public:

    SensorStatus(uint status = 0) {
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

  private:
      
    bool light_uv_failed      : 1;                              // 0b100000000
    bool precip_failed        : 1;                              // 0b010000000
    bool wind_failed          : 1;                              // 0b001000000
    bool rh_failed            : 1;                              // 0b000100000
    bool temperature_failed   : 1;                              // 0b000010000
    bool pressure_failed      : 1;                              // 0b000001000
    bool lightning_disturber  : 1;                              // 0b000000100
    bool lightning_noise      : 1;                              // 0b000000010
    bool lightning_failed     : 1;                              // 0b000000001
  };

  UdpDeviceStatus(const Json& event, bool& ok) {
    ok = false;
    memset(this, 0, sizeof(*this));

    const string& device_id = serial_number(event);   
    const string& hub_id = hub_sn(event);
    double device_version = firmware_revision(event);
    double epoch = event["timestamp"].number_value();

    if (!device_id.empty() && !hub_id.empty() && epoch) {
      uptime_ = event["uptime"].number_value();
      battery_ = event["voltage"].number_value();
      device_rssi_ = event["rssi"].number_value();
      hub_rssi_ = event["hub_rssi"].number_value(); 
      sensor_status_ = event["sensor_status"].number_value();
      debug_ = event["debug"].number_value();

      ok = Init(device_status, epoch, hub_id.c_str(), 0, device_id.c_str(), device_version);
    }
  }

private:

  int uptime_;
  double battery_;
  int device_rssi_;
  int hub_rssi_; 
  SensorStatus sensor_status_;
  bool debug_;
};

// Hub Status --------------------------------------------------
//
// {
//   "serial_number":"HB-00000001",
//   "type":"hub_status",
//   "firmware_revision":"35",
//   "uptime":1670133,                                          hub uptime (seconds)
//   "rssi":-62,                                                hub RSSI (dB)
//   "timestamp":1495724691,
//   "reset_flags":"BOR,PIN,POR",                               see tempest::UdpHubStatus::Reset
//   "seq":48,                                                  undocumented
//   "fs":[                                                     undocumented
//     1,
//     0,
//     15675411,
//     524288
//   ],
//   "radio_stats":[                                            
//     2,                                                       version
//     1,                                                       reboot count
//     0,                                                       I2C bus error count
//     3                                                        0b00) Off, 0b01) On, 0b11) Active
//   ],
//   "mqtt_stats":[                                             undocumented
//     1,
//     0
//   ]
// }
//
// "{\"serial_number\":\"HB-00000001\",\"type\":\"hub_status\",\"firmware_revision\":\"35\",\"uptime\":1670133,\"rssi\":-62,\"timestamp\":1495724691,\"reset_flags\":\"BOR,PIN,POR\",\"seq\":48,\"fs\":[1,0,15675411,524288],\"radio_stats\":[2,1,0,3],\"mqtt_stats\":[1,0]}"
// 

class UdpHubStatus: UdpEvent {
public:

  class ResetFlags {
  public:

    ResetFlags(const Json* event = nullptr)  {
      BOR = PIN = POR = SFT = WDG = WWD = LPW = false;

      if (event) {
        // split comma separated string
        stringstream flags{(*event)["reset_flags"].string_value()};
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

  private:
  
    bool BOR                  : 1;                              // 0b001000000 Brownout reset
    bool PIN                  : 1;                              // 0b000100000 PIN reset
    bool POR                  : 1;                              // 0b000010000 Power reset
    bool SFT                  : 1;                              // 0b000001000 Software reset
    bool WDG                  : 1;                              // 0b000000100 Watchdog reset
    bool WWD                  : 1;                              // 0b000000010 Window watchdog reset
    bool LPW                  : 1;                              // 0b000000001 Low-power reset  
  };

  UdpHubStatus(const Json& event, bool& ok) {
    ok = false;
    memset(this, 0, sizeof(*this));

    const string& hub_id = serial_number(event);   
    double hub_version = firmware_revision(event);
    double epoch = event["timestamp"].number_value();

    if (!hub_id.empty() && epoch) {
      uptime_ = event["uptime"].number_value();
      rssi_ = event["rssi"].number_value();       

      reset_ = ResetFlags(&event);
      seq_ = event["seq"].number_value();

      const Json::array& fs = event["fs"].array_items();
      fs_[0] = fs[0].number_value();
      fs_[1] = fs[1].number_value();
      fs_[2] = fs[2].number_value();
      fs_[3] = fs[3].number_value();        

      const Json::array& radio = event["radio_stats"].array_items();
      radio_version_ = radio[0].number_value();
      radio_reboot_count_ = radio[1].number_value();
      radio_i2c_bus_err_count_ = radio[2].number_value();
      radio_ = (RadioStatus)radio[3].number_value();

      const Json::array& mqtt = event["mqtt_stats"].array_items();
      mqtt_[0] = mqtt[0].number_value();
      mqtt_[1] = mqtt[1].number_value();

      ok = Init(hub_status, epoch, hub_id.c_str(), hub_version, "", 0);
    }
  }

private:

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
};

// -------------------------------------------------------------

class Rest {
public:
private:

};

class Ecowitt {
public:
private:

};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

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

  Json jevent;
  string err;
  bool ok;

  for (int i = 0; i < (sizeof(test_json) / sizeof(test_json[0])); i++) {

    jevent = Json::parse(test_json[i], err);

    if (jevent == nullptr) {
      cout << "JSON parsing error: " << err << endl;      
    }
    else {
      const string& type = jevent["type"].string_value();
      if (type == "evt_precip") {
        UdpRainStart event{jevent, ok};
        cout << "Event evt_precip: " << ok << endl;  
      }
      else if (type == "evt_strike") {  
        UdpLightningStrike event{jevent, ok};      
        cout << "Event evt_strike: " << ok << endl;          
      }
      else if (type == "rapid_wind") {  
        UdpRapidWind event{jevent, ok};      
        cout << "Event rapid_wind: " << ok << endl;          
      }
      else if (type == "obs_air") {        
        size_t size;
        int idx = 0;

        do {
          UdpAirObservation event{jevent, idx, size, ok};
          cout << "Event obs_air: " << ok << endl;
        } while (++idx < size);    
      }
      else if (type == "obs_sky") {  
        size_t size;
        int idx = 0;

        do {
          UdpSkyObservation event{jevent, idx, size, ok};
          cout << "Event obs_sky: " << ok << endl;
        } while (++idx < size);
      }
      else if (type == "obs_st") {   
        size_t size;
        int idx = 0;

        do {
          UdpTempestObservation event{jevent, idx, size, ok};
          cout << "Event obs_st: " << ok << endl;          
        } while (++idx < size);
      }
      else if (type == "device_status") {   
        UdpDeviceStatus event{jevent, ok};     
        cout << "Event device_status: " << ok << endl;          
      }
      else if (type == "hub_status") {
        UdpHubStatus event{jevent, ok};  
        cout << "Event hub_status: " << ok << endl;          
      }
      else if (type.find("debug") == string::npos) {
        cout << "Unrecognized event: " << type << endl;
      }
      else {
        cout << "Skipped debug: " << type << endl;      
      }
    }
  }

  return (0);
}

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_API
