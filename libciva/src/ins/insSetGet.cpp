#include "ins/ins.h"

namespace libciva {

void INS::advanceActionMalfunctionIndex() noexcept {
  uint8_t index = (uint8_t)std::max(1, (displayActionMalfunctionCodeIndex + 1) % 8);
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
    index = (uint8_t)std::max(1, (index + 1) % 8);

    if (index == displayActionMalfunctionCodeIndex) {
      if (actionMalfunctionCodes.value == 0) displayActionMalfunctionCodeIndex = 0;
      break;
    }
  }
}

const POSITION INS::currentNavPosition(const double dTime) noexcept {
  if (currentTripleMixPosition.isValid()) {
    POSITION p =
        currentINSPosition + (currentTripleMixPosition - currentINSPosition) * std::min(1.0, mixEaseTime / MIX_EASE_TIME);
    mixEaseTime += dTime;
    return p;
  } else {
    mixEaseTime = 0;
    return currentINSPosition;
  }
}

void INS::calculateTrack() noexcept {
  double trueHeading = varManager.sim.planeHeadingDegreesTrue;
  double windDir = varManager.sim.ambientWindDirection;
  double windSpeed = varManager.sim.ambientWindVelocity;
  double tas = varManager.sim.airspeedTrue;

  if (groundSpeed < MIN_GS) {
    track = trueHeading;
    return;
  }

  double _trueHeading = trueHeading * DEG2RAD;
  double _windDir = std::fmod(windDir + 180, 360) * DEG2RAD;

  track = std::fmod(360 + (std::atan2(tas * std::sin(_trueHeading) + windSpeed * std::sin(_windDir),
                                      tas * std::cos(_trueHeading) + windSpeed * std::cos(_windDir))) *
                              RAD2DEG,
                    360);
}

void INS::exportVars() const noexcept {
  VarManager::UnitExport &unit = varManager.unit[(uint8_t)unitIndex];
  // For UI
  unit.displayLeft = display.value & 0x00000000FFFFFFFF;
  unit.displayRight = (display.value & 0xFFFFFFFF00000000) >> 32;
  unit.indicators = indicators.value;
  // For UX
  unit.modeSelectorPos = (uint8_t)modeSelector;
  unit.dataSelectorPos = (uint8_t)dataSelector;
  unit.waypointSelectorPos = waypointSelector;
  unit.autoMode = autoMode;
  // For systems consumption
  unit.crossTrackError = crossTrackError;
  unit.desiredTrack = desiredTrack;
  unit.track = track;
  unit.trackAngleError = trackAngleError;
  unit.distance = remainingDistance;
  unit.gs = groundSpeed;
  unit.valid = (uint8_t)valid;
}

} // namespace libciva