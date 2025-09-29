#ifndef GEOUTILS_H
#define GEOUTILS_H

#define _USE_MATH_DEFINES

#include <math.h>

double distanceInNMI(double startLat, double startLon, double endLat, double endLon);

bool isPosValid(double lat, double lon);

bool isPosInLimit(double lat);

#endif