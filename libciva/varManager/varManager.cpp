#include "varManager.h"

void VarManager::setVar(const std::string &name, double value) noexcept {
  store[name] = value;
}

bool VarManager::getVar(const std::string &name, double &value) const noexcept {
  auto it = store.find(name);

  if (it != store.end()) {
    value = it->second;
    return true;
  }

  return false;
}
