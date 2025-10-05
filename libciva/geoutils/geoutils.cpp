#include "geoutils.h"

double POSITION::distanceTo(const POSITION &target) const noexcept {
  double R = 3440; // nmi
  double φ1 = latitude * M_PI / 180; // φ, λ in radians
  double φ2 = target.latitude * M_PI / 180;
  double Δφ = (target.latitude - latitude) * M_PI / 180;
  double Δλ = (target.longitude - longitude) * M_PI / 180;

  double a = sin(Δφ / 2) * sin(Δφ / 2) + cos(φ1) * cos(φ2) * sin(Δλ / 2) * sin(Δλ / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return R * c; // in nmi
}


