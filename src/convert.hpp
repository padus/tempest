//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/mircolino/tempest
//
// Description: metric <--> imperial conversions
//

#ifndef TEMPEST_CONVERSIONS
#define TEMPEST_CONVERSIONS

// Includes --------------------------------------------------------------------------------------------------------------------

#include "system.hpp"

// Source ----------------------------------------------------------------------------------------------------------------------

namespace tempest {

using namespace std;

class Convert {
public:

  // -----------------------------------------------------------

  inline static double F_to_C(double val) {
    return ((val - 32) / 1.8);
  }

  // -----------------------------------------------------------

  inline static double C_to_F(double val) {
    return ((val * 1.8) + 32);
  }

  // -----------------------------------------------------------

  inline static double inHg_to_hPa(double val) {
    return (val * 33.863886666667);
  }

  // -----------------------------------------------------------

  inline static double hPa_to_inHg(double val) {
    return (val / 33.863886666667);
  }

  // -----------------------------------------------------------

  inline static double in_to_mm(double val) {
    return (val * 25.4);
  }

  // -----------------------------------------------------------

  inline static double mm_to_in(double val) {
    return (val / 25.4);
  }

  // -----------------------------------------------------------

  inline static double ms_to_kmh(double val) {
    return (val / 0.27777777777778);
  }

  // -----------------------------------------------------------// -----------------------------------------------------------

  inline static double ft_to_m(double val) {
    return (val / 3.28084);
  }

  // -----------------------------------------------------------

  inline static double m_to_ft(double val) {
    return (val * 3.28084);
  }

  // -----------------------------------------------------------

  inline static double mi_to_km(double val) {
    return (val * 1.609344);
  }

  // -----------------------------------------------------------

  inline static double km_to_mi(double val) {
    return (val / 1.609344);
  }

  // -----------------------------------------------------------

  inline static double Wm2_to_lux(double val) {
    return (val / 0.0079);
  }

  // -----------------------------------------------------------

  inline static double lux_to_Wm2(double val) {
    return (val * 0.0079);
  }

  // -----------------------------------------------------------

  inline static string epoch_to_dateutc(time_t epoch) {

    char buf[128];
    tm *ts = gmtime(&epoch);
    strftime(buf, sizeof(buf), "%Y-%m-%d+%H:%M:%S", ts);

    string date = buf;

    return (date);
  }

};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_CONVERSIONS
