#include "ins/ins.h"

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
  double trueHeading = 0;
  double windDir = 0;
  double windSpeed = 0;
  double tas = 0;
  double gs = 0;

  if (varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading) &&
      varManager.getVar(SIM_VAR_AMBIENT_WIND_DIRECTION, windDir) && varManager.getVar(SIM_VAR_AMBIENT_WIND_VELOCITY, windSpeed) &&
      varManager.getVar(SIM_VAR_AIRSPEED_TRUE, tas) && varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs)) {

    if (gs < MIN_GS) {
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
}

void INS::exportVars() const noexcept {
  varManager.setVar(DISPLAY_VAR + id, display.value);
  varManager.setVar(INDICATORS_VAR + id, indicators.value);
  varManager.setVar(MODE_SELECTOR_POS_VAR + id, (double)modeSelector);
  varManager.setVar(DATA_SELECTOR_POS_VAR + id, (double)dataSelector);
  varManager.setVar(WAYPOINT_SELECTOR_POS_VAR + id, (double)waypointSelector);
  varManager.setVar(AUTO_MAN_POS_VAR + id, (double)autoMode);
  varManager.setVar(CROSS_TRACK_ERROR_VAR + id, crossTrackError);
  varManager.setVar(DESIRED_TRACK_VAR + id, desiredTrack);
  varManager.setVar(DISTANCE_VAR + id, remainingDistance);
  varManager.setVar(VALID + id, (double)valid);
}
