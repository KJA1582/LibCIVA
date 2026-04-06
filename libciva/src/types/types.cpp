#include "types/types.h"

#define R 3440.0 // nmi
namespace libciva {

double POSITION::distanceTo(const POSITION &target) const noexcept {
  double φ1 = latitude * DEG2RAD; // φ, λ in radians
  double φ2 = target.latitude * DEG2RAD;
  double Δφ = (target.latitude - latitude) * DEG2RAD;
  double Δλ = (target.longitude - longitude) * DEG2RAD;

  double a = std::sin(Δφ / 2) * std::sin(Δφ / 2) + std::cos(φ1) * std::cos(φ2) * std::sin(Δλ / 2) * std::sin(Δλ / 2);
  double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

  return R * c; // in nmi
}

double POSITION::bearingTo(const POSITION &target) const noexcept {
  double φ1 = latitude * DEG2RAD;
  double λ1 = longitude * DEG2RAD;
  double φ2 = target.latitude * DEG2RAD;
  double λ2 = target.longitude * DEG2RAD;

  double y = std::sin(λ2 - λ1) * std::cos(φ2);
  double x = std::cos(φ1) * std::sin(φ2) - std::sin(φ1) * std::cos(φ2) * std::cos(λ2 - λ1);
  double θ = std::atan2(y, x);

  return std::fmod(θ * RAD2DEG + 360.0, 360.0); // in degrees
}

double POSITION::crossTrackDistance(const POSITION &start, const POSITION &end) const noexcept {
  double d13 = start.distanceTo(*this);
  double θ13 = start.bearingTo(*this) * DEG2RAD;
  double θ12 = start.bearingTo(end) * DEG2RAD;

  double δ13 = d13 / R;
  return std::asin(std::sin(δ13) * std::sin(θ13 - θ12)) * R;
}

double POSITION::alongTrackDistance(const POSITION &start, const POSITION &end) const noexcept {
  double d13 = start.distanceTo(*this);
  double θ13 = start.bearingTo(*this) * DEG2RAD;
  double θ12 = start.bearingTo(end) * DEG2RAD;

  double δ13 = d13 / R;
  double dXt = std::asin(std::sin(δ13) * std::sin(θ13 - θ12));
  return std::acos(std::cos(δ13) / std::cos(dXt)) * R;
}

std::ostream &operator<<(std::ostream &os, const POSITION &dt) {
  os << dt.latitude << ',' << dt.longitude;
  return os;
}

POSITION POSITION::destination(const double distance, const double bearing) const noexcept {
  double φ = latitude * DEG2RAD;
  double λ = longitude * DEG2RAD;
  double θ = bearing * DEG2RAD;

  double φ2 = std::asin(std::sin(φ) * std::cos(distance / R) + std::cos(φ) * std::sin(distance / R) * std::cos(θ));
  double λ2 =
      λ + std::atan2(std::sin(θ) * std::sin(distance / R) * std::cos(φ), std::cos(distance / R) - std::sin(φ) * std::sin(φ2));

  return {φ2 * RAD2DEG, std::fmod((λ2 * RAD2DEG) + 540.0, 360.0) - 180.0};
}

void POSITION::bound() noexcept {
  if (latitude > 90) latitude = 90 - (latitude - 90);
  if (latitude < -90) latitude = -90 - (90 + latitude);

  if (longitude > 180) longitude = 180 - (longitude - 180);
  if (longitude < -180) longitude = -180 - (180 + longitude);
}

bool POSITION::inFront(const POSITION &pos, const double track) const noexcept {
  double brg = pos.bearingTo(*this);
  return std::fabs(std::fmod(brg - track + 540, 360) - 180) < 90;
}

POSITION POSITION::intersection(const double bearing1, const POSITION &pos, const double bearing2) noexcept {
  double φ1 = latitude * DEG2RAD;
  double λ1 = longitude * DEG2RAD;
  double φ2 = pos.latitude * DEG2RAD;
  double λ2 = pos.longitude * DEG2RAD;
  double θ13 = bearing1 * DEG2RAD;
  double θ23 = bearing2 * DEG2RAD;
  double Δφ = φ2 - φ1;
  double Δλ = λ2 - λ1;

  // angular distance p1-p2
  double δ12 = 2 * std::asin(std::sqrt(std::sin(Δφ / 2) * std::sin(Δφ / 2) +
                                       std::cos(φ1) * std::cos(φ2) * std::sin(Δλ / 2) * std::sin(Δλ / 2)));
  if (std::fabs(δ12) < EPSILON) return *this; // coincident points

  // initial/final bearings between points
  double cosθa = (std::sin(φ2) - std::sin(φ1) * std::cos(δ12)) / (std::sin(δ12) * std::cos(φ1));
  double cosθb = (std::sin(φ1) - std::sin(φ2) * std::cos(δ12)) / (std::sin(δ12) * std::cos(φ2));
  double θa = std::acos(std::fmin(std::fmax(cosθa, -1.0), 1.0)); // protect against rounding errors
  double θb = std::acos(std::fmin(std::fmax(cosθb, -1.0), 1.0)); // protect against rounding errors

  double θ12 = std::sin(λ2 - λ1) > 0 ? θa : 2 * PI - θa;
  double θ21 = std::sin(λ2 - λ1) > 0 ? 2 * PI - θb : θb;

  double α1 = θ13 - θ12; // angle 2-1-3
  double α2 = θ21 - θ23; // angle 1-2-3

  if (std::sin(α1) == 0 && std::sin(α2) == 0) return {999, 999}; // infinite intersections
  if (std::sin(α1) * std::sin(α2) < 0) return {999, 999};        // ambiguous intersection (antipodal/360°)

  double cosα3 = -std::cos(α1) * std::cos(α2) + std::sin(α1) * std::sin(α2) * std::cos(δ12);

  double δ13 = std::atan2(std::sin(δ12) * std::sin(α1) * std::sin(α2), std::cos(α2) + std::cos(α1) * cosα3);

  double φ3 =
      std::asin(std::fmin(std::fmax(std::sin(φ1) * std::cos(δ13) + std::cos(φ1) * std::sin(δ13) * std::cos(θ13), -1.0), 1.0));

  double Δλ13 = std::atan2(std::sin(θ13) * std::sin(δ13) * std::cos(φ1), std::cos(δ13) - std::sin(φ1) * std::sin(φ3));
  double λ3 = λ1 + Δλ13;

  return {φ3 * RAD2DEG, λ3 * RAD2DEG};
}

} // namespace libciva