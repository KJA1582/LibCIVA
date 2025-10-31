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

void INS::updateCurrentINSPosition(const double dTime) noexcept {
  double simLat = 999;
  double simLon = 999;
  varManager.getVar(SIM_VAR_PLANE_LATITUDE, simLat);
  varManager.getVar(SIM_VAR_PLANE_LONGITUDE, simLon);
  POSITION simPos = { simLat, simLon };

  if (!simPos.isValid() || initialTimeInNAV == 0) return;
  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> disRad(0.0, 360.0);
  
  std::normal_distribution<> disDist(0);

  static double rad = disRad(gen);
  // Increase spread based on timeInNav, scaled by dT
  // Spread is constructed so that sum over many dT adheres to the 3sigma of the unit
  // Ex: timeInNav is 10800 (3h), dTime is 3600 (1h) -> spread of 3
  // Ex: timeInNav is 3600 (1h), dTime is 0.1s (100ms) -> spread 0.00002777777
  double err = std::abs(disDist(gen));
  double initialDist = err * (initialTimeInNAV / 3600) * (dTime / 3600);
  double dist = err * (timeInNAV / 3600) * (dTime / 3600);

  initialError = initialError.destination(initialDist, rad);
  currentError = currentError.destination(dist, rad);

  currentINSPosition = simPos + simPosDelta + currentError;
  currentINSPosition.bound();
  initialINSPosition = simPos + simPosDelta + initialError;
  initialINSPosition.bound();
}
