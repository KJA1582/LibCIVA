#include "types/types.h"

constexpr double R = 3440; // nmi

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
  double brg = this->bearingTo(pos);
  return std::abs(std::fmod(brg - track + 540, 360) - 180) < 90;
}

void POSITION_VECTOR::normalize() noexcept {
  double mag = std::sqrt(this->dot(*this));
  x = x / mag;
  y = y / mag;
  z = z / mag;
}

POSITION_VECTOR POSITION_VECTOR::fromPosition(POSITION &position) noexcept {
  double lat = roundCoord(position.latitude * DEG2RAD);
  double lon = roundCoord(position.longitude * DEG2RAD);

  return {std::cos(lat) * std::cos(lon), std::cos(lat) * std::sin(lon), std::sin(lat)};
}

POSITION POSITION_VECTOR::toPosition() const noexcept {
  return {roundCoord(std::asin(z) * RAD2DEG), roundCoord(std::atan2(y, x) * RAD2DEG)};
}