#ifndef VAR_MANAGER_H
#define VAR_MANAGER_H

#include <cstdint>
#include <map>
#include <string>

constexpr auto VAR_START = "LIBCIVA_";

constexpr auto DISPLAY_VAR = "DISPLAY_";
constexpr auto INDICATORS_VAR = "INDICATORS_";
constexpr auto DATA_SELECTOR_POS_VAR = "DATA_SELECTOR_POS_";
constexpr auto MODE_SELECTOR_POS_VAR = "MODE_SELECTOR_POS_";
constexpr auto WAYPOINT_SELECTOR_POS_VAR = "WAYPOINT_SELECTOR_POS_";

constexpr auto SIM_VAR_AMBIENT_TEMPERATURE = "AMBIENT TEMPERATURE";
constexpr auto SIM_VAR_AMBIENT_WIND_DIRECTION = "AMBIENT WIND DIRECTION";
constexpr auto SIM_VAR_AMBIENT_WIND_VELOCITY = "AMBIENT WIND VELOCITY";
constexpr auto SIM_VAR_AIRSPEED_TRUE = "AIRSPEED TRUE";
constexpr auto SIM_VAR_GROUND_VELOCITY = "GROUND VELOCITY";
constexpr auto SIM_VAR_PLANE_HEADING_DEGREES_TRUE = "PLANE HEADING DEGREES TRUE";

class VarManager {
  const std::string prefix;

protected:
  std::map<std::string, double> store;

public:
  VarManager(const std::string &prefix) noexcept : prefix(prefix) { }
  virtual ~VarManager() noexcept { }

  virtual void setVar(const std::string &name, double value) noexcept;
  virtual bool getVar(const std::string &name, double &value) const noexcept;
};

#endif

