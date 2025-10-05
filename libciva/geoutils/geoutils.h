#ifndef GEOUTILS_H
#define GEOUTILS_H

#define _USE_MATH_DEFINES

#include <cmath>

struct POSITION {
  // In degrees
  double latitude;
  // In degrees
  double longitude;

  inline bool isValid() const noexcept {
    return latitude <= 90 && latitude >= -90 && longitude <= 180 && longitude >= -180;
  }

  double distanceTo(const POSITION &target) const noexcept;

};

#endif