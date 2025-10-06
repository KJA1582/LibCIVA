#include "types.h"

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
