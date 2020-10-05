//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: system log c++ stream wrapper
//

#ifndef TEMPEST_LOG
#define TEMPEST_LOG

// Includes --------------------------------------------------------------------------------------------------------------------

#include "system.hpp"

// Source ----------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

enum log_level {
  emergency = LOG_EMERG,
  alert     = LOG_ALERT,
  critical  = LOG_CRIT,
  error     = LOG_ERR,
  warning   = LOG_WARNING,
  notice    = LOG_NOTICE,
  info      = LOG_INFO,
  debug     = LOG_DEBUG,
};

enum log_facility {
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

class log_streambuf: public streambuf {
public:

  ~log_streambuf() override {
    closelog();
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
  friend class log_stream;

  explicit log_streambuf(log_facility fac, log_level lev): streambuf() {
    // Private constructor so only log_stream can create us
    set_facility(fac);
    set_level(lev);
    openlog(nullptr, LOG_PID, fac);
  }

  inline void set_level(log_level lev) noexcept {
    level_ = lev;
    level_stream_ = lev;
    level_mask_ = LOG_UPTO(lev);
    setlogmask(level_mask_);
  }

  inline log_level get_level(void) const noexcept {
    return (level_);
  }

  inline void set_facility(log_facility fac) noexcept {
    facility_ = fac;
  }

  inline log_facility get_facility(void) const noexcept {
    return (facility_);
  }

  inline bool is_lev_enabled(log_level lev) const noexcept {
   return (level_mask_ & LOG_MASK(lev));
  }

  inline void set_level_stream(log_level lev) noexcept {
    // Only called by the operator <<
    level_stream_ = lev;
  }

  string buffer_;
  log_facility facility_;                                       // facility
  log_level level_;                                             // level
  log_level level_stream_;                                      // operator << level
  int level_mask_;

  static const char* const level_tag_[];                        // see initialization below
};

const char* const log_streambuf::level_tag_[] = {
  "[EMERG]",
  "[ALERT]",
  " [CRIT]",
  "[ERROR]",
  " [WARN]",
  " [NOTE]",
  " [INFO]",
  "[DEBUG]"
};

//
// syslog c++ stream wrapper
//
// Usage:
//
// log_stream log{log_facility::user, log_level::info};
//
// log << log_level::error << "This is an error" << endl;
//
// TLOG_WARNING(log)  << "This is a warning" << endl;
//

class log_stream: public ostream {
public:

  explicit log_stream(log_facility fac = log_facility::user, log_level lev = log_level::info): ostream(&streambuf), streambuf(fac, lev) {}

  inline void set_level(log_level lev) noexcept { streambuf.set_level(lev); }
  inline log_level get_level(void) const noexcept { return (streambuf.get_level()); }

  inline void set_facility(log_facility fac) noexcept { streambuf.set_facility(fac); }
  inline log_facility get_facility(void) const noexcept { return (streambuf.get_facility()); }

  inline bool is_lev_enabled(log_level lev) const noexcept { return (streambuf.is_lev_enabled(lev)); }

  log_stream& operator<<(log_level lev) noexcept {
    streambuf.set_level_stream(lev);
    return (*this);
  }

private:

  log_streambuf streambuf;
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

#define TLOG(OBJ, LEVEL)        (OBJ.is_lev_enabled(LEVEL)) && (OBJ << LEVEL << "[" << __FILENAME__ << ":" << __func__ << ":" << __LINE__ << "] ")

#define TLOG_EMERG(OBJ)         TLOG(OBJ, log_level::emergency)
#define TLOG_ALERT(OBJ)         TLOG(OBJ, log_level::alert)
#define TLOG_CRIT(OBJ)          TLOG(OBJ, log_level::critical)
#define TLOG_ERROR(OBJ)         TLOG(OBJ, log_level::error)
#define TLOG_WARNING(OBJ)       TLOG(OBJ, log_level::warning)
#define TLOG_NOTICE(OBJ)        TLOG(OBJ, log_level::notice)
#define TLOG_INFO(OBJ)          TLOG(OBJ, log_level::info)
#define TLOG_DEBUG(OBJ)         TLOG(OBJ, log_level::debug)

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_LOG
