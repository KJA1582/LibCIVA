#include "ins.h"

void INS::advanceActionMalfunctionIndex() noexcept {
  uint8_t index = std::max(1, (displayActionMalfunctionCodeIndex + 1) % 8);
  while (true) {
    switch (index) {
      case 1: {
        if (actionMalfunctionCodes.codes.A02_31) {
          displayActionMalfunctionCodeIndex = 1;
          return;
        }
        break;
      }
      case 2: {
        if (actionMalfunctionCodes.codes.A02_42) {
          displayActionMalfunctionCodeIndex = 2;
          return;
        }
        break;
      }
      case 3: {
        if (actionMalfunctionCodes.codes.A02_49) {
          displayActionMalfunctionCodeIndex = 3;
          return;
        }
        break;
      }
      case 4: {
        if (actionMalfunctionCodes.codes.A02_63) {
          displayActionMalfunctionCodeIndex = 4;
          return;
        }
        break;
      }
      case 5: {
        if (actionMalfunctionCodes.codes.A04_41) {
          displayActionMalfunctionCodeIndex = 5;
          return;
        }
        break;
      }
      case 6: {
        if (actionMalfunctionCodes.codes.A04_43) {
          displayActionMalfunctionCodeIndex = 6;
          return;
        }
        break;
      }
      case 7: {
        if (actionMalfunctionCodes.codes.A04_57) {
          displayActionMalfunctionCodeIndex = 7;
          return;
        }
        break;
      }
    }
    index = std::max(1, (index + 1) % 8);

    if (index == displayActionMalfunctionCodeIndex) {
      displayActionMalfunctionCodeIndex = index != 5 ? index : 0;
      break;
    }
  }
}

void INS::updateSimPosDelta() noexcept {
  double simLat = 999;
  double simLon = 999;
  varManager.getVar(SIM_VAR_PLANE_LATITUDE, simLat);
  varManager.getVar(SIM_VAR_PLANE_LONGITUDE, simLon);
  POSITION simPos = { simLat, simLon };

  if (simPos.isValid()) {
    simPosDelta = currentINSPosition - simPos;
  }
}