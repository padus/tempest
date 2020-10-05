//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: system log c++ stream wrapper
// Inspired by: https://stackoverflow.com/a/28886535/1043426
//

#ifndef TEMPEST_LOG
#define TEMPEST_LOG

// Includes --------------------------------------------------------------------------------------------------------------------

#include "system.hpp"

// Source ----------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

//
// syslog c++ stream wrapper
//
// Usage:
//
// Log log{Log::facility::user, Log::level::info};
//
// log << Log::level::error << "This is an error" << endl;
//
// TLOG_WARNING(log)  << "This is a warning" << endl;
//

class Log: public ostream {
public:

  enum Level {
    emergency = LOG_EMERG,
    alert     = LOG_ALERT,
    critical  = LOG_CRIT,
    error     = LOG_ERR,
    warning   = LOG_WARNING,
    notice    = LOG_NOTICE,
    info      = LOG_INFO,
    debug     = LOG_DEBUG,
  };

  enum Facility {
    auth      = LOG_AUTH,
    cron      = LOG_CRON,
    daemon    = LOG_DAEMON,
    local0    = LOG_LOCAL0,
    local1    = LOG_LOCAL1,
    local2    = LOG_LOCAL2,
    local3    = LOG_LOCAL3,
    local4    = LOG_LOCAL4,
    local5    = LOG_LOCAL5,
    local6    = LOG_LOCAL6,
    local7    = LOG_LOCAL7,
    print     = LOG_LPR,
    mail      = LOG_MAIL,
    news      = LOG_NEWS,
    user      = LOG_USER,
    uucp      = LOG_UUCP,
  };

  explicit Log(Facility fac = Facility::user, Level lev = Level::info): ostream(&buf_), buf_(fac, lev) {}

  inline void SetLevel(Level lev) noexcept { buf_.set_level(lev); }
  inline Level GetLevel(void) const noexcept { return (buf_.get_level()); }

  inline void SetFacility(Facility fac) noexcept { buf_.set_facility(fac); }
  inline Facility GetFacility(void) const noexcept { return (buf_.get_facility()); }

  inline bool IsLevelEnabled(Level lev) const noexcept { return (buf_.is_lev_enabled(lev)); }

  Log& operator<<(Level lev) noexcept {
    buf_.set_level_stream(lev);
    return (*this);
  }

private:

  class log_buf: public streambuf {
  public:

    explicit log_buf(Facility fac, Level lev) {
      // Private constructor so only log_stream can create us
      set_facility(fac);
      set_level(lev);
      openlog(nullptr, LOG_PID, fac);
    }

    ~log_buf() override {
      closelog();
    }

    inline void set_level(Level lev) noexcept {
      level_ = lev;
      level_stream_ = lev;
      level_mask_ = LOG_UPTO(lev);
      setlogmask(level_mask_);
    }

    inline Level get_level(void) const noexcept {
      return (level_);
    }

    inline void set_facility(Facility fac) noexcept {
      facility_ = fac;
    }

    inline Facility get_facility(void) const noexcept {
      return (facility_);
    }

    inline bool is_lev_enabled(Level lev) const noexcept {
    return (level_mask_ & LOG_MASK(lev));
    }

    inline void set_level_stream(Level lev) noexcept {
      // Only called by the operator <<
      level_stream_ = lev;
    }

  protected:

    int_type overflow(int_type c = traits_type::eof()) override {
      if (traits_type::eq_int_type(c, traits_type::eof())) sync();
      else buffer_ += traits_type::to_char_type(c);

      return (c);
    }

    int sync(void) override {
      if (buffer_.size()) {
        syslog(LOG_MAKEPRI(facility_, level_stream_), "%s%s", level_tag_[level_stream_], buffer_.c_str());

        buffer_.clear();

        // Reset operator << level to default in case next time log_level is not present
        level_stream_ = level_;
      }

      return (0);
    }

  private:

    string buffer_;
    Facility facility_;                                           // facility
    Level level_;                                                 // level
    Level level_stream_;                                          // operator << level
    int level_mask_;

    static const char* const level_tag_[];                        // see initialization below
  }
  buf_;
};

const char* const Log::log_buf::level_tag_[] = {
  "[EMERG]",
  "[ALERT]",
  " [CRIT]",
  "[ERROR]",
  " [WARN]",
  " [NOTE]",
  " [INFO]",
  "[DEBUG]"
};

static constexpr const char* past_last_slash(const char* const str) {
  //
  // Remove path from __FILE__ macro
  //
  const char* last_slash = str;

  for (const char* pos = str; *pos != '\0'; ++pos) {
    if (*pos == '/') last_slash = pos + 1;
  }

  return (last_slash);
}

#define __FILENAME__            ({constexpr const char* const sf__ {past_last_slash(__FILE__)}; sf__;})

#define TLOG(OBJ, LEVEL)        (OBJ.IsLevelEnabled(LEVEL)) && (OBJ << LEVEL << "[" << __FILENAME__ << ":" << __func__ << ":" << __LINE__ << "] ")

#define TLOG_EMERG(OBJ)         TLOG(OBJ, Log::Level::emergency)
#define TLOG_ALERT(OBJ)         TLOG(OBJ, Log::Level::alert)
#define TLOG_CRIT(OBJ)          TLOG(OBJ, Log::Level::critical)
#define TLOG_ERROR(OBJ)         TLOG(OBJ, Log::Level::error)
#define TLOG_WARNING(OBJ)       TLOG(OBJ, Log::Level::warning)
#define TLOG_NOTICE(OBJ)        TLOG(OBJ, Log::Level::notice)
#define TLOG_INFO(OBJ)          TLOG(OBJ, Log::Level::info)
#define TLOG_DEBUG(OBJ)         TLOG(OBJ, Log::Level::debug)

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_LOG
