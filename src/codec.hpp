//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: Tempest data codec
// API:         https://weatherflow.github.io/SmartWeather/api/udp.html
//

#ifndef TEMPEST_CODEC
#define TEMPEST_CODEC

// Includes -------------------------------------------------------------------------------------------------------------------

#include "system.hpp"

#include "log.hpp"
#include "convert.hpp"

// Source ---------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Sensor {
public:

  enum Model {
    UNKNOWN = 0,
    AIR = 1,
    SKY = 2,
    TEMPEST = 3
  };

  enum Precipitation {
    NONE = 0,
    RAIN = 1,
    HAIL = 2,
  };

  class Status {
  public:

    Status(int status = 0) {
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

  Sensor(const string& id, size_t queue_max): id_{id}, model_{GetModel(id)}, queue_max_{queue_max} {

    memset(&precipitation_, 0, sizeof(precipitation_));
    memset(&lightning_, 0, sizeof(lightning_));
    memset(&wind_, 0, sizeof(wind_));
    memset(&obs_, 0, sizeof(obs_));
    memset(&status_, 0, sizeof(status_));
    memset(&obs_stats_, 0, sizeof(obs_stats_));
    memset(&event_stats_, 0, sizeof(event_stats_));
  }

  size_t UdpPrecipitation(const Json& event) {
    const Json::array& evt = event["evt"].array_items();

    precipitation_.timestamp = evt[0].number_value();

    obs_stats_.PrecipitationStarted(precipitation_.timestamp);

    event_stats_.precipitation++;
    return (1);
  }

  size_t UdpLightning(const Json& event) {
    const Json::array& evt = event["evt"].array_items();

    lightning_.timestamp = evt[0].number_value();
    lightning_.distance = evt[1].number_value();
    lightning_.energy = evt[2].number_value();

    event_stats_.lightning++;
    return (1);
  }

  size_t UdpWind(const Json& event) {
    const Json::array& evt = event["ob"].array_items();

    wind_.timestamp = evt[0].number_value();
    wind_.speed = evt[1].number_value();
    wind_.direction = evt[2].number_value();

    event_stats_.wind++;
    return (1);
  }

  size_t UdpObservationAir(const Json& event) {
    // We can have a vector of observations (WF developers confirmed oldest is first in the array)
    const Json::array& obs = event["obs"].array_items();
    size_t idx, size = obs.size();

    obs_.version = event["firmware_revision"].number_value();

    for (idx = 0; idx < size; idx++) {
      const Json::array& evt = obs[idx].array_items();

      obs_.timestamp = evt[0].number_value();
      obs_.pressure = evt[1].number_value();
      obs_.temperature = evt[2].number_value();
      obs_.humidity = evt[3].number_value();
      obs_.lightning_count = evt[4].number_value();
      obs_.lightning_distance = evt[5].number_value();
      obs_.battery = evt[6].number_value();
      obs_.timespan = evt[7].number_value() * 60;

      event_stats_.observation++;
    }

    return (idx);
  }

  size_t UdpObservationSky(const Json& event) {
    // We can have a vector of observations (WF developers confirmed oldest is first in the array)
    const Json::array& obs = event["obs"].array_items();
    size_t idx, size = obs.size();

    obs_.version = event["firmware_revision"].number_value();

    for (idx = 0; idx < size; idx++) {
      const Json::array& evt = obs[idx].array_items();

      obs_.timestamp = evt[0].number_value();
      obs_.illuminance = evt[1].number_value();
      obs_.uv = evt[2].number_value();
      obs_.precipitation_accumulation = evt[3].number_value();
      obs_.wind_lull = evt[4].number_value();
      obs_.wind_speed = evt[5].number_value();
      obs_.wind_gust = evt[6].number_value();
      obs_.wind_direction = evt[7].number_value();
      obs_.battery = evt[8].number_value();
      obs_.timespan = evt[9].number_value() * 60;
      obs_.solar_radiation = evt[10].number_value();
      // obs_.precipitation_daily_accumulation = evt[11].number_value();
      obs_.precipitation_type = (Precipitation)evt[12].number_value();
      obs_.wind_sample = evt[13].number_value();

      obs_stats_.Update(obs_.timestamp, obs_.timespan, obs_.precipitation_accumulation, obs_.wind_direction, obs_.wind_speed, obs_.wind_gust);
      event_stats_.observation++;
    }

    return (idx);
  }

  size_t UdpObservationTempest(const Json& event) {
    // We can have a vector of observations (WF developers confirmed oldest is first in the array)
    const Json::array& obs = event["obs"].array_items();
    size_t idx, size = obs.size();

    obs_.version = event["firmware_revision"].number_value();

    for (idx = 0; idx < size; idx++) {
      const Json::array& evt = obs[idx].array_items();

      obs_.timestamp = evt[0].number_value();
      obs_.wind_lull = evt[1].number_value();
      obs_.wind_speed = evt[2].number_value();
      obs_.wind_gust = evt[3].number_value();
      obs_.wind_direction = evt[4].number_value();
      obs_.wind_sample = evt[5].number_value();
      obs_.pressure = evt[6].number_value();
      obs_.temperature = evt[7].number_value();
      obs_.humidity = evt[8].number_value();
      obs_.illuminance = evt[9].number_value();
      obs_.uv = evt[10].number_value();
      obs_.solar_radiation = evt[11].number_value();
      obs_.precipitation_accumulation = evt[12].number_value();
      obs_.precipitation_type = (Precipitation)evt[13].number_value();
      obs_.lightning_distance = evt[14].number_value();
      obs_.lightning_count = evt[15].number_value();
      obs_.battery = evt[16].number_value();
      obs_.timespan = evt[17].number_value() * 60;

      obs_stats_.Update(obs_.timestamp, obs_.timespan, obs_.precipitation_accumulation, obs_.wind_direction, obs_.wind_speed, obs_.wind_gust);
      event_stats_.observation++;
    }

    return (idx);
  }

  size_t UdpStatus(const Json& event) {
    status_.timestamp = event["timestamp"].number_value();
    status_.uptime = event["uptime"].number_value();
    status_.battery = event["voltage"].number_value();
    status_.version = event["firmware_revision"].number_value();
    status_.rssi = event["rssi"].number_value();
    // status_.hub_rssi_ = event["hub_rssi"].number_value();
    status_.status = event["sensor_status"].number_value();
    status_.debug = event["debug"].number_value();

    event_stats_.status++;
    return (1);
  }

  const string id_;
  const Model model_;
  const size_t queue_max_;

  // Rain Start Event
  struct {
    time_t timestamp;
  }
  precipitation_;

  // Lightning Strike Event
  struct {
    time_t timestamp;
    double distance;
    double energy;
  }
  lightning_;

  // Rapid Wind Event
  struct {
    time_t timestamp;
    double speed;
    double direction;
  }
  wind_;

  // Observation Event
  struct {
    time_t timestamp;
    int timespan;

    int version;
    double battery;
    double temperature;
    double humidity;
    double pressure;

    double illuminance;
    double uv;
    double solar_radiation;

    double precipitation_accumulation;                         // precipitation accumulation during the time span
    Precipitation precipitation_type;                          // precipitation type

    double lightning_distance;
    int lightning_count;

    double wind_speed;
    double wind_lull;
    double wind_gust;
    double wind_direction;
    int wind_sample;                                           // sample over which gust and lull are calculated (in seconds)
  }
  obs_;

  // Device Status Event
  struct {
    time_t timestamp;
    int uptime;
    double battery;
    int version;
    int rssi;

    Status status;
    bool debug;
  }
  status_;

  // Observation statistics
  struct {
    struct tm track;            // time tracking

    double precip_rate;         // mm/h
    double precip_event;        // mm
    double precip_hourly;       // mm
    double precip_daily;        // mm
    double precip_weekly;       // mm
    double precip_monthly;      // mm
    double precip_yearly;       // mm
    double precip_total;        // mm

    double wind_direction;
    double wind_direction_avg10m;
    double wind_speed;
    double wind_speed_avg10m;
    double wind_gust;
    double wind_gust_daily;

    double wind_sample[2][10];  // 10m wind direction and speed samples
    size_t wind_index;          // 0-9

    void PrecipitationStarted(time_t time) {
      // A rain start event arrived and it's not raining: add a minimal amount just to signal it
      // next observation should then report things correclty
      if (!precip_rate) precip_rate = 0.01;
    }

    void Update(time_t time, int span, double level, double direction, double speed, double gust) {
      // Roll-over time
      struct tm roll = *gmtime(&time);

      if (roll.tm_year != track.tm_year) {
        track = roll;
        precip_hourly = precip_daily = precip_weekly = precip_monthly = precip_yearly = 0;
        wind_gust_daily = 0;
      }
      else if (roll.tm_mon != track.tm_mon) {
        track = roll;
        precip_hourly = precip_daily = precip_weekly = precip_monthly = 0;
        wind_gust_daily = 0;
      }
      else if (roll.tm_wday == 0 && track.tm_wday == 6) {
        track = roll;
        precip_hourly = precip_daily = precip_weekly = 0;
        wind_gust_daily = 0;
      }
      else if (roll.tm_yday != track.tm_yday) {
        track = roll;
        precip_hourly = precip_daily = 0;
        wind_gust_daily = 0;
      }
      else if (roll.tm_hour != track.tm_hour) {
        track = roll;
        precip_hourly = 0;
      }

      // Precipitation stats
      precip_event = precip_rate? (precip_event + level): level;
      precip_hourly += level;
      precip_daily += level;
      precip_weekly += level;
      precip_monthly += level;
      precip_yearly += level;
      precip_total += level;

      precip_rate = span? ((3600 / span) * level): 0;

      // Wind stats
      wind_direction = direction;
      wind_speed = speed;
      wind_gust = gust;
      wind_gust_daily = max(gust, wind_gust_daily);

      span = (span < 60)? 1: (span / 60);
      while (span--) {
        wind_sample[0][wind_index] = direction;
        wind_sample[1][wind_index] = speed;

        wind_index = (wind_index + 1) % 10;
      }
      
      Convert::wind_vector_to_avg(wind_sample[0], wind_sample[1], 10, wind_direction_avg10m, wind_speed_avg10m);
    }
  }
  obs_stats_;

  // Event Statistics
  struct {
    uint precipitation;
    uint lightning;
    uint wind;
    uint observation;
    uint status;
  }
  event_stats_;

private:

  static Model GetModel(const string& id) {
    if (id.find("AR-") == 0) return (Model::AIR);
    if (id.find("SK-") == 0) return (Model::SKY);
    if (id.find("ST-") == 0) return (Model::TEMPEST);
    return (Model::UNKNOWN);
  }
};

class Hub {
public:

  enum Radio {
    OFF = 0,
    ON = 1,
    ACTIVE = 3
  };

  class ResetFlags {
  public:

    ResetFlags(const string& list = empty_string) {
      BOR = PIN = POR = SFT = WDG = WWD = LPW = false;

      if (!list.empty()) {
        // Split comma separated string
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

  static string Model(const string& id) {
    return ("WF-HB01");
  }

  Hub(const string& id, size_t queue_max): id_{id}, model_{Model(id)}, queue_max_{queue_max} {

    memset(&status_, 0, sizeof(status_));
    memset(&event_stats_, 0, sizeof(event_stats_));
  }

  Sensor& GetSensor(const string& sensor_id) {
    size_t idx;

    assert(!sensor_id.empty());

    for (idx = 0; idx < sensor_.size(); idx++) {
      if (sensor_[idx].id_ == sensor_id) return (sensor_[idx]);
    }

    sensor_.emplace_back(sensor_id, queue_max_);
    return (sensor_[idx]);
  }

  size_t UdpStatus(const Json& event) {

    status_.version = strtod(event["firmware_revision"].string_value().c_str(), nullptr);
    status_.timestamp = event["timestamp"].number_value();

    status_.uptime = event["uptime"].number_value();
    status_.rssi = event["rssi"].number_value();

    status_.reset = ResetFlags(event["reset_flags"].string_value());
    status_.seq = event["seq"].number_value();

    const Json::array& fs = event["fs"].array_items();
    status_.fs[0] = fs[0].number_value();
    status_.fs[1] = fs[1].number_value();
    status_.fs[2] = fs[2].number_value();
    status_.fs[3] = fs[3].number_value();

    const Json::array& radio = event["radio_stats"].array_items();
    status_.radio_version = radio[0].number_value();
    status_.radio_reboot_count = radio[1].number_value();
    status_.radio_i2c_bus_err_count = radio[2].number_value();
    status_.radio = (Radio)radio[3].number_value();

    const Json::array& mqtt = event["mqtt_stats"].array_items();
    status_.mqtt[0] = mqtt[0].number_value();
    status_.mqtt[1] = mqtt[1].number_value();

    event_stats_.status++;
    return (1);
  }

  const string id_;
  const string model_;
  const size_t queue_max_;

  vector<Sensor> sensor_;

  struct {
    time_t timestamp;

    int version;
    int uptime;
    int rssi;
    ResetFlags reset;
    int seq;
    int fs[4];

    int radio_version;
    int radio_reboot_count;
    int radio_i2c_bus_err_count;

    Radio radio;

    int mqtt[2];
  }
  status_;

  // Event Statistics
  struct {
    uint status;
  }
  event_stats_;
};

class Tempest {
public:

  Tempest(size_t queue_max = 128): start_time_{time(nullptr)}, queue_max_{queue_max} {
    memset(&event_stats_, 0, sizeof(event_stats_));
  }

  string StatsUdp(void) const {
    ostringstream stats{""};
    size_t hubs, sensors;

    double uptime = difftime(time(nullptr), start_time_);
    int days = uptime / 86400;
    uptime -= days * 86400;
    int hours = uptime / 3600;
    uptime -= hours * 3600;
    int minutes = uptime / 60;
    uptime -= minutes * 60;
    int seconds = uptime;

    stats << "Uptime: " << days << "d." << hours << "h." << minutes << "m." << seconds << "s" << endl;
    stats << "Invalid Events: " << event_stats_.invalid << endl;
    stats << "Debug Events: " << event_stats_.debug << endl;
    stats << "Unknown Events: " << event_stats_.unknown << endl;
    hubs = hub_.size();
    stats << "Hubs: " << hubs << endl;
    for (size_t i = 0; i < hubs; i++ ) {
      const Hub& hub = hub_[i];
      stats << "[" << i << "]: " << hub.id_ << " " << hub.status_.version << endl;
      stats << "     Status Events: " << hub.event_stats_.status << endl;
      sensors = hub.sensor_.size();
      stats << "     Sensors: " << sensors << endl;
      for (size_t i = 0; i < sensors; i++ ) {
        const Sensor& sensor = hub.sensor_[i];
        stats << "     [" << i << "]: " << sensor.id_ << " " << sensor.status_.version << endl;
        stats << "          Rain Start Events: " << sensor.event_stats_.precipitation << endl;
        stats << "          Lightning Strike Events: " << sensor.event_stats_.lightning << endl;
        stats << "          Rapid wind Events: " << sensor.event_stats_.wind << endl;
        stats << "          Observation Events: " << sensor.event_stats_.observation << endl;
        stats << "          Status Events: " << sensor.event_stats_.status << endl;
      }
    }

    return (stats.str());
  }

  size_t WriteUdp(Log& log, const char udp[], size_t udp_len, bool& notify) {
    //
    // Return the number of events/observation written to tempest
    // or 0 if error/debug/unrecognized
    //
    size_t obs = 0;
    notify = false;

    string err;
    Json event = Json::parse(udp, err);
    if (event == nullptr) {
      event_stats_.invalid++;
      TLOG_ERROR(log) << "JSON error: " << err << " parsing: " << udp << "." << endl;
    }
    else {
      const string& type = event["type"].string_value();

      if (type == "hub_status") {
        Hub& hub = GetHub(event["serial_number"].string_value());
        obs = hub.UdpStatus(event);
      }
      else {
        Sensor& sensor = GetHub(event["hub_sn"].string_value()).GetSensor(event["serial_number"].string_value());

        if (type == "evt_precip") {
          obs = sensor.UdpPrecipitation(event);
          if (obs > 0) notify = true;
        }
        else if (type == "evt_strike") {
          obs = sensor.UdpLightning(event);
          if (obs > 0) notify = true;
        }
        else if (type == "rapid_wind") {
          obs = sensor.UdpWind(event);
        }
        else if (type == "obs_air") {
          obs = sensor.UdpObservationAir(event);
        }
        else if (type == "obs_sky") {
          obs = sensor.UdpObservationSky(event);
        }
        else if (type == "obs_st") {
          obs = sensor.UdpObservationTempest(event);
        }
        else if (type == "device_status") {
          obs = sensor.UdpStatus(event);
        }
        else if (type.find("debug") != string::npos) {
          event_stats_.debug++;
        }
        else {
          event_stats_.unknown++;
          TLOG_WARNING(log) << "Unrecognized UDP event: " << udp << "." << endl;
        }
      }
    }

    return (obs);
  }

  size_t ReadEcowitt(Log& log, vector<string>& data) {
    //
    // Return the number of events/observation read from tempest
    // or 0 if error
    //
    data.clear();

    ostringstream event;
    size_t hubs, sensors;
    string ch;

    hubs = hub_.size();
    for (size_t i = 0; i < hubs; i++ ) {
      Hub& hub = hub_[i];
      sensors = hub.sensor_.size();
      for (size_t i = 0; i < sensors; i++ ) {
        Sensor& sensor = hub.sensor_[i];
        ch = "_wf" + std::to_string(i + 1) + "=";

          event.str("");

          // Hub attributes (head)
          event << "PASSKEY=" << hub.id_;
          event << "&stationtype=" << hub.model_ << "_V" << hub.status_.version << ".0.0";
          event << "&dateutc=" << Convert::epoch_to_dateutc(hub.status_.timestamp);

          // Sensor attributes
          event << "&batt" << ch << sensor.obs_.battery;

        if (sensor.model_ == Sensor::Model::AIR || sensor.model_ == Sensor::Model::TEMPEST) {
          // Temperature, humidity and pressure
          event << "&tempf" << ch << Convert::C_to_F(sensor.obs_.temperature);
          event << "&humidity" << ch << sensor.obs_.humidity;
          event << "&baromrelin" << ch << "0";
          event << "&baromabsin" << ch << Convert::hPa_to_inHg(sensor.obs_.pressure);

          // Lightning: if we got a strike after the last observation we temporarely increase the count
          if (sensor.lightning_.timestamp > sensor.obs_.timestamp) sensor.obs_.lightning_count++;
          event << "&lightning" << ch << sensor.lightning_.distance;
          event << "&lightning_time" << ch << sensor.lightning_.timestamp;
          event << "&lightning_energy" << ch << sensor.lightning_.energy;
          event << "&lightning_num" << ch << sensor.obs_.lightning_count;
        }

        if (sensor.model_ == Sensor::Model::SKY || sensor.model_ == Sensor::Model::TEMPEST) {
          // Solar
          event << "&uv" << ch << sensor.obs_.uv;
          event << "&solarradiation" << ch << sensor.obs_.solar_radiation;

          // Precipitation
          event << "&rainratein" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_rate);
          event << "&eventrainin" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_event);
          event << "&hourlyrainin" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_hourly);
          event << "&dailyrainin" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_daily);
          event << "&weeklyrainin" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_weekly);
          event << "&monthlyrainin" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_monthly);
          event << "&yearlyrainin" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_yearly);
          event << "&totalrainin" << ch << Convert::mm_to_in(sensor.obs_stats_.precip_total);

          // Wind
          event << "&winddir" << ch << sensor.obs_stats_.wind_direction;
          event << "&winddir_avg10m" << ch << sensor.obs_stats_.wind_direction_avg10m;
          event << "&windspeedmph" << ch << Convert::km_to_mi(Convert::ms_to_kmh(sensor.obs_stats_.wind_speed));
          event << "&windspdmph_avg10m" << ch << Convert::km_to_mi(Convert::ms_to_kmh(sensor.obs_stats_.wind_speed_avg10m));
          event << "&windgustmph" << ch << Convert::km_to_mi(Convert::ms_to_kmh(sensor.obs_stats_.wind_gust));
          event << "&maxdailygust" << ch << Convert::km_to_mi(Convert::ms_to_kmh(sensor.obs_stats_.wind_gust_daily));
        }

          // Hub attributes (tail)
          event << "&freq=RSSI" << hub.status_.rssi;
          event << "&model=" << hub.model_;

        if (!(event.str().empty())) data.emplace_back(event.str());
      }
    }

    return (data.size());
  }

private:

  Hub& GetHub(const string& hub_id) {
    size_t idx;

    assert(!hub_id.empty());

    for (idx = 0; idx < hub_.size(); idx++) {
      if (hub_[idx].id_ == hub_id) return (hub_[idx]);
    }

    hub_.emplace_back(hub_id, queue_max_);
    return (hub_[idx]);
  }

  const time_t start_time_;
  const size_t queue_max_;

  vector<Hub> hub_;

  struct {
    uint debug;
    uint unknown;
    uint invalid;
  }
  event_stats_;
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

#endif // TEMPEST_CODEC
