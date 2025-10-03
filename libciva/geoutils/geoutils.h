#ifndef GEOUTILS_H
#define GEOUTILS_H

#define _USE_MATH_DEFINES

#include <cmath>

#include "varManager/varManager.h"

double distanceInNMI(double startLat, double startLon, double endLat, double endLon) noexcept;

bool isPosValid(double lat, double lon) noexcept;

#endif