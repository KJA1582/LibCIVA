#ifndef VAR_MANAGER_H
#define VAR_MANAGER_H

// For MSFS so IntelliSense doesn't crap out
#ifndef __INTELLISENSE__
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include <cstdint>

namespace libciva {

constexpr auto DISPLAY_VAR = "LIBCIVA_DISPLAY_";
constexpr auto INDICATORS_VAR = "LIBCIVA_INDICATORS_";
constexpr auto DATA_SELECTOR_POS_VAR = "LIBCIVA_DATA_SELECTOR_POS_";
constexpr auto MODE_SELECTOR_POS_VAR = "LIBCIVA_MODE_SELECTOR_POS_";
constexpr auto WAYPOINT_SELECTOR_POS_VAR = "LIBCIVA_WAYPOINT_SELECTOR_POS_";
constexpr auto AUTO_MAN_POS_VAR = "LIBCIVA_AUTO_MAN_POS_";
constexpr auto CROSS_TRACK_ERROR_VAR = "LIBCIVA_CROSS_TRACK_ERROR_";
constexpr auto DESIRED_TRACK_VAR = "LIBCIVA_DESIRED_TRACK_";
constexpr auto TRACK_VAR = "LIBCIVA_TRACK_";
constexpr auto TRACK_ANGLE_ERROR_VAR = "LIBCIVA_TRACK_ANGLE_ERROR_";
constexpr auto DISTANCE_VAR = "LIBCIVA_DISTANCE_";
constexpr auto GROUND_SPEED_VAR = "LIBCIVA_GROUND_SPEED_";
constexpr auto VALID = "LIBCIVA_VALID_";

constexpr auto SIM_VAR_AIRSPEED_TRUE = "AIRSPEED TRUE";
constexpr auto SIM_VAR_AMBIENT_TEMPERATURE = "AMBIENT TEMPERATURE";
constexpr auto SIM_VAR_AMBIENT_WIND_DIRECTION = "AMBIENT WIND DIRECTION";
constexpr auto SIM_VAR_AMBIENT_WIND_VELOCITY = "AMBIENT WIND VELOCITY";
constexpr auto SIM_VAR_PLANE_HEADING_DEGREES_TRUE = "PLANE HEADING DEGREES TRUE";
constexpr auto SIM_VAR_PLANE_LATITUDE = "PLANE LATITUDE";
constexpr auto SIM_VAR_PLANE_LONGITUDE = "PLANE LONGITUDE";
constexpr auto SIM_VAR_NAV_DME_1 = "NAV DME:1";
constexpr auto SIM_VAR_NAV_DME_2 = "NAV DME:2";
constexpr auto SIM_VAR_SIMULATION_RATE = "SIMULATION RATE";
constexpr auto SIM_VAR_PLANE_ALTITUDE = "PLANE ALTITUDE";
constexpr auto SIM_VAR_VELOCITY_WORLD_X = "VELOCITY WORLD X";
constexpr auto SIM_VAR_VELOCITY_WORLD_Z = "VELOCITY WORLD Z";
constexpr auto SIM_VAR_ACCELERATION_WORLD_X = "ACCELERATION WORLD X";
constexpr auto SIM_VAR_ACCELERATION_WORLD_Z = "ACCELERATION WORLD Z";

struct VarManager {
  struct UnitExport {
    double display = 0;
    double indicators = 0;
    double modeSelectorPos = 0;
    double dataSelectorPos = 0;
    double waypointSelectorPos = 0;
    double autoManPos = 0;
    double crossTrackError = 0;
    double desiredTrack = 0;
    double track = 0;
    double trackAngleError = 0;
    double distance = 0;
    double gs = 0;
    double valid = 0;
  };

  struct SimVars {
    double ambientTemperature = 15;
    double ambientWindDirection = 0;
    double ambientWindVelocity = 0;
    double airspeedTrue = 0;
    double groundVelocity = 0;
    double planeHeadingDegreesTrue = 0;
    double planeLatitude = 0;
    double planeLongitude = 0;
    double navDme1 = -1;
    double navDme2 = -1;
    double simulationRate = 1;
    double planeAltitude = 0;
    double velocityWorldX = 0;
    double velocityWorldZ = 0;
    double accelWorldX = 0;
    double accelWorldZ = 0;
  } sim;

  UnitExport unit[3];
};

} // namespace libciva

#endif
