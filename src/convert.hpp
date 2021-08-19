//
// App:         WeatherFlow Tempest UDP Relay
// Author:      Mirco Caramori
// Copyright:   (c) 2020 Mirco Caramori
// Repository:  https://github.com/padus/tempest
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

  // -----------------------------------------------------------

  inline static double degree_to_radian(double degree) {
    return (degree * (M_PI / 180)); 
  } 

  // -----------------------------------------------------------

  inline static double radian_to_degree(double radian) {
    return (radian * (180 / M_PI)); 
  } 

  // -----------------------------------------------------------

  inline static void wind_vector_to_avg(const double direction[], const double speed[], size_t size, double& direction_avg, double& speed_avg) {
    //
    // based on https://www.researchgate.net/publication/262766424_Technical_note_Averaging_wind_speeds_and_directions
    //
    double sin_sum = 0, cos_sum = 0;

    for (size_t i = 0; i < size; i++) {
      sin_sum += -speed[i] * sin(2 * M_PI * direction[i] / 360);
      cos_sum += -speed[i] * cos(2 * M_PI * direction[i] / 360);
    }

    // Average
    sin_sum /= size;
    cos_sum /= size;

    //Simple Pythagorean Theorem
    speed_avg = sqrt((sin_sum * sin_sum) + (cos_sum * cos_sum));

    direction_avg = (atan2(sin_sum, cos_sum) * 360 / 2 / M_PI) + 180;
  }
};

} // namespace tempest

// Recycle Bin ----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF ------------------------------------------------------------------------------------------------------------------------

#endif // TEMPEST_CONVERSIONS
