#include "types/types.h"

constexpr double R = 3440; // nmi

double POSITION::distanceTo(const POSITION &target) const noexcept {
  double φ1 = latitude * M_PI / 180; // φ, λ in radians
  double φ2 = target.latitude * M_PI / 180;
  double Δφ = (target.latitude - latitude) * M_PI / 180;
  double Δλ = (target.longitude - longitude) * M_PI / 180;

  double a = std::sin(Δφ / 2) * std::sin(Δφ / 2) + std::cos(φ1) * std::cos(φ2) * std::sin(Δλ / 2) * std::sin(Δλ / 2);
  double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

  return R * c; // in nmi
}

double POSITION::bearingTo(const POSITION &target) const noexcept {
  double φ1 = latitude * M_PI / 180;
  double λ1 = longitude * M_PI / 180;
  double φ2 = target.latitude * M_PI / 180;
  double λ2 = target.longitude * M_PI / 180;

  double y = std::sin(λ2 - λ1) * std::cos(φ2);
  double x = std::cos(φ1) * std::sin(φ2) - std::sin(φ1) * std::cos(φ2) * std::cos(λ2 - λ1);
  double θ = std::atan2(y, x);

  return std::fmod(θ * 180.0 / M_PI + 360.0, 360.0); // in degrees
}

double POSITION::crossTrackDistance(const POSITION &start, const POSITION &end) const noexcept {
  double d13 = start.distanceTo(*this);
  double θ13 = start.bearingTo(*this) * M_PI / 180;
  double θ12 = start.bearingTo(end) * M_PI / 180;

  double δ13 = d13 / R;
  return std::asin(std::sin(δ13) * std::sin(θ13 - θ12)) * R;
}

double POSITION::alongTrackDistance(const POSITION &start, const POSITION &end) const noexcept {
  double d13 = start.distanceTo(*this);
  double θ13 = start.bearingTo(*this) * M_PI / 180;
  double θ12 = start.bearingTo(end) * M_PI / 180;

  double δ13 = d13 / R;
  double dXt = std::asin(std::sin(δ13) * std::sin(θ13 - θ12));
  return std::acos(std::cos(δ13) / std::cos(dXt)) * R;
}

POSITION POSITION::destination(const double distance, const double bearing) const noexcept {
  double φ = latitude * M_PI / 180;
  double λ = longitude * M_PI / 180;
  double θ = bearing * M_PI / 180;

  double φ2 = std::asin(std::sin(φ) * std::cos(distance / R) +
                        std::cos(φ) * std::sin(distance / R) * std::cos(θ));
  double λ2 = λ + std::atan2(std::sin(θ) * std::sin(distance / R) * std::cos(φ),
                             std::cos(distance / R) - std::sin(φ) * std::sin(φ2));

  return { φ2 * 180.0 / M_PI, std::fmod((λ2 * 180.0 / M_PI) + 540.0, 360.0) - 180.0 };
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