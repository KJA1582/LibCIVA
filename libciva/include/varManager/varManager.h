#ifndef VAR_MANAGER_H
#define VAR_MANAGER_H

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
#include <map>
#include <string>

constexpr auto DISPLAY_VAR = "LIBCIVA_DISPLAY_";
constexpr auto INDICATORS_VAR = "LIBCIVA_INDICATORS_";
constexpr auto DATA_SELECTOR_POS_VAR = "LIBCIVA_DATA_SELECTOR_POS_";
constexpr auto MODE_SELECTOR_POS_VAR = "LIBCIVA_MODE_SELECTOR_POS_";
constexpr auto WAYPOINT_SELECTOR_POS_VAR = "LIBCIVA_WAYPOINT_SELECTOR_POS_";
constexpr auto AUTO_MAN_POS_VAR = "LIBCIVA_AUTO_MAN_POS_";
constexpr auto CROSS_TRACK_ERROR_VAR = "LIBCIVA_CROSS_TRACK_ERROR_";
constexpr auto DESIRED_TRACK_VAR = "LIBCIVA_DESIRED_TRACK_";

constexpr auto SIM_VAR_AIRSPEED_TRUE = "AIRSPEED TRUE";
constexpr auto SIM_VAR_AMBIENT_TEMPERATURE = "AMBIENT TEMPERATURE";
constexpr auto SIM_VAR_AMBIENT_WIND_DIRECTION = "AMBIENT WIND DIRECTION";
constexpr auto SIM_VAR_AMBIENT_WIND_VELOCITY = "AMBIENT WIND VELOCITY";
constexpr auto SIM_VAR_GROUND_VELOCITY = "GROUND VELOCITY";
constexpr auto SIM_VAR_PLANE_HEADING_DEGREES_TRUE = "PLANE HEADING DEGREES TRUE";
constexpr auto SIM_VAR_PLANE_LATITUDE = "PLANE LATITUDE";
constexpr auto SIM_VAR_PLANE_LONGITUDE = "PLANE LONGITUDE";
constexpr auto SIM_VAR_NAV_DME_1 = "NAV DME:1";
constexpr auto SIM_VAR_NAV_DME_2 = "NAV DME:2";
constexpr auto SIM_VAR_SIMULATION_RATE = "SIMULATION RATE";

class VarManager {
protected:
  std::map<std::string, double> store;

public:
  VarManager() noexcept;
  virtual ~VarManager() noexcept {}

  virtual void setVar(const std::string &name, double value) noexcept;
  virtual bool getVar(const std::string &name, double &value) const noexcept;
};

#endif
