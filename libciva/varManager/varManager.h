#ifndef VAR_MANAGER_H
#define VAR_MANAGER_H

#include <string>
#include <cstdint>
#include <map>

constexpr auto VAR_START = "LIBCIVA_";

constexpr auto DATA_SELECTOR_POS_VAR = "DATA_SELECTOR_POS_";
constexpr auto MODE_SELECTOR_POS_VAR = "MODE_SELECTOR_POS_";
constexpr auto STATE_VAR = "STATE_";
constexpr auto ALIGN_SUBMODE_VAR = "ALIGN_SUBMODE_";
constexpr auto ACCURACY_INDEX_VAR = "ACCURACY_INDEX_";
constexpr auto TEMPERATURE_VAR = "TEMPERATURE_";
constexpr auto OPERATING_TIME_VAR = "OPERATING_TIME_";
constexpr auto DISPLAY_POS_LAT_VAR = "LAT_DISPLAY_POS_";
constexpr auto DISPLAY_POS_LON_VAR = "LON_DISPLAY_POS_";
constexpr auto INS_POS_LAT_VAR = "LAT_INS_POS_";
constexpr auto INS_POS_LON_VAR = "LON_INS_POS_";
constexpr auto ACTION_MALFUNCTION_CODE_VAR = "ACTION_MALFUNCTION_CODE_";
constexpr auto INDICATORS_VAR = "INDICATORS_";
constexpr auto BATTERY_TEST_VAR = "BATTERY_TEST_";
constexpr auto DISPLAY_VAR = "DISPLAY_";
constexpr auto INSERT_MODE_VAR = "INSERT_MODE_";
constexpr auto WPT_SELECTOR_POS_VAR = "WPT_SELECTOR_POS_";
constexpr auto WPT_POS_LAT_VAR = "LAT_WPT_";
constexpr auto WPT_POS_LON_VAR = "LON_WPT_";

constexpr auto SIM_VAR_AMBIENT_TEMPERATURE = "AMBIENT TEMPERATURE";
constexpr auto SIM_VAR_AMBIENT_WIND_DIRECTION = "AMBIENT WIND DIRECTION";
constexpr auto SIM_VAR_AMBIENT_WIND_VELOCITY = "AMBIENT WIND VELOCITY";
constexpr auto SIM_VAR_AIRSPEED_TRUE = "AIRSPEED TRUE";
constexpr auto SIM_VAR_GROUND_VELOCITY = "GROUND VELOCITY";

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

