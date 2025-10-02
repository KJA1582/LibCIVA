#include "geoutils.h"

double distanceInNMI(double startLat, double startLon, double endLat, double endLon) {
  double R = 3440; // nmi
  double φ1 = startLat * M_PI / 180; // φ, λ in radians
  double φ2 = endLat * M_PI / 180;
  double Δφ = (endLat - startLat) * M_PI / 180;
  double Δλ = (endLon - startLon) * M_PI / 180;

  double a = sin(Δφ / 2) * sin(Δφ / 2) + cos(φ1) * cos(φ2) * sin(Δλ / 2) * sin(Δλ / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return R * c; // in nmi
}

bool isPosValid(double lat, double lon) {
  return lat <= 90 && lat >= -90 && lon <= 180 && lon >= -180;
}
