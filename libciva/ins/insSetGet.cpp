#include "ins.h"

#pragma region Action Malfunction Code

void INS::addActionMalfunctionCode(const ACTION_MALFUNCTION_CODE code) noexcept {
  auto end = actionMalfunctionCodes.end();

  if (std::find(actionMalfunctionCodes.begin(), end, code) == end) {
    actionMalfunctionCodes.push_back(code);
  }
}
bool INS::removeCurrentActionMalfunctionCode() noexcept {
  actionMalfunctionCodes.erase(actionMalfunctionCodes.begin() + displayActionMalfunctionCodeIndex);

  return actionMalfunctionCodes.empty();
}
void INS::clearActionMalfunctionCodes() noexcept {
  actionMalfunctionCodes.clear();
}
bool INS::hasActionMalfunctionCode(const ACTION_MALFUNCTION_CODE code) const noexcept {
  auto end = actionMalfunctionCodes.end();

  return std::find(actionMalfunctionCodes.begin(), end, code) != end;
}
bool INS::hasActionMalfunctionCode() const noexcept {
  return !actionMalfunctionCodes.empty();
}
void INS::incCurrentActionMalfunctionCode() noexcept {
  if (displayActionMalfunctionCodeIndex == actionMalfunctionCodes.size() - 1) {
    displayActionMalfunctionCodeIndex = 0;
  }
  else {
    displayActionMalfunctionCodeIndex++;
  }
}
ACTION_MALFUNCTION_CODE INS::getCurrentActionMalfunctionCode() const noexcept {
  if (actionMalfunctionCodes.empty()) return ACTION_MALFUNCTION_CODE::INV;

  return actionMalfunctionCodes[displayActionMalfunctionCodeIndex];
}

#pragma endregion
