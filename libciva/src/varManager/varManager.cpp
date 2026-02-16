#include "varManager/varManager.h"

namespace libciva {

VarManager::VarManager() noexcept {
  setVar(SIM_VAR_AMBIENT_TEMPERATURE, 15);
  setVar(SIM_VAR_AMBIENT_WIND_DIRECTION, 0);
  setVar(SIM_VAR_AMBIENT_WIND_VELOCITY, 0);
  setVar(SIM_VAR_AIRSPEED_TRUE, 0);
  setVar(SIM_VAR_GROUND_VELOCITY, 0);
  setVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, 0);
  setVar(SIM_VAR_PLANE_LATITUDE, 0);
  setVar(SIM_VAR_PLANE_LONGITUDE, 0);
  setVar(SIM_VAR_NAV_DME_1, -1);
  setVar(SIM_VAR_NAV_DME_2, -1);
  setVar(SIM_VAR_SIMULATION_RATE, 1);
}

void VarManager::setVar(const std::string &name, double value) noexcept { store[name] = value; }

bool VarManager::getVar(const std::string &name, double &value) const noexcept {
  auto it = store.find(name);

  if (it != store.end()) {
    value = it->second;
    return true;
  }

  return false;
}

} // namespace libciva