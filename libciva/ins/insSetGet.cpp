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
  double groundSpeed = 0;
  varManager.getVar(SIM_VAR_PLANE_LATITUDE, simLat);
  varManager.getVar(SIM_VAR_PLANE_LONGITUDE, simLon);

  POSITION simPos = { simLat, simLon };

  if (!simPos.isValid() || initialTimeInNAV == 0 || !varManager.getVar(SIM_VAR_GROUND_VELOCITY, groundSpeed)) return;
  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> disRad(0.0, 360.0);

  std::normal_distribution<> disDist(0);

  static double rad = disRad(gen);
  // Increase spread based on timeInNav, scaled by dT
  // Spread is constructed so that sum over many dT adheres to the 3sigma of the unit
  // Ex: timeInNav is 10800 (3h), dTime is 3600 (1h) -> spread of 3
  // Ex: timeInNav is 3600 (1h), dTime is 0.1s (100ms) -> spread 0.00002777777
  double err = std::abs(disDist(gen)) * std::min(0.1, groundSpeed / DRIFT_GS);
  double initialDist = err * (initialTimeInNAV / 3600) * (dTime / 3600);
  double dist = err * (timeInNAV / 3600) * (dTime / 3600);

  initialError = initialError.destination(initialDist, rad);
  currentError = currentError.destination(dist, rad);

  currentINSPosition = simPos + simPosDelta + currentError;
  currentINSPosition.bound();
  initialINSPosition = simPos + simPosDelta + initialError;
  initialINSPosition.bound();
}

void INS::updateMetrics() noexcept {
  double gs;
  bool gsValid = varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs);
  double trueHeading;
  bool trueHeadingValid = varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading);

  uint16_t _track = 0;
  if (!gsValid) {
    gs = 0;

    if (trueHeadingValid || gs < MIN_GS) {
      _track = (uint16_t)(std::round(trueHeading));
    }
  }
  else {
    _track = (uint16_t)(std::round(track));
  }

  double alongDist = currentINSPosition.alongTrackDistance(waypoints[currentLegStart], waypoints[currentLegEnd]);
  double legCrs = waypoints[currentLegStart].bearingTo(waypoints[currentLegEnd]);
  POSITION alongPos = waypoints[currentLegStart].destination(alongDist, legCrs);

  /* XTK */

  crossTrackError = currentINSPosition.crossTrackDistance(waypoints[currentLegStart], waypoints[currentLegEnd]);

  /* DSRTK */

  if (alongDist < waypoints[currentLegStart].distanceTo(waypoints[currentLegEnd])) {
    desiredTrack = alongPos.bearingTo(waypoints[currentLegEnd]);
  }
  else {
    desiredTrack = legCrs;
  }
}

void INS::updateNav(const double dTime) noexcept {
  // Skip if not in auto
  if (!autoMode) return;

  double gs;
  bool gsValid = varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs);

  // Invalid/slow speed, skip
  if (!gsValid || gs <= 1) return;

  double crsToEnd = currentINSPosition.bearingTo(waypoints[currentLegEnd]);
  double nextCrs = waypoints[currentLegEnd].bearingTo(waypoints[(currentLegEnd % 9) + 1]);
  double dist = currentINSPosition.distanceTo(waypoints[currentLegEnd]);
  double legTime = (waypoints[currentLegStart].distanceTo(waypoints[currentLegEnd]) / gs) * 3600;

  // Leg time less than min and not yet flown min leg time
  if (legTime < MIN_LEG_TIME || timeInLeg < MIN_LEG_TIME) {
    timeInLeg += dTime;
    return;
  }

  double turnRadius = (gs * gs) / (11.26 * std::tan(config.getExpectedBankAngle() * M_PI / 180.0));
  double delta = nextCrs - track;
  while (delta < -180.0) delta += 360.0;
  while (delta >= 180.0) delta -= 360.0;
  double turnDist = turnRadius * std::tan((std::abs(delta) * M_PI / 180.0) / 2.0);
  turnDist /= 6076.1154856;

  // Turn point hit, advance leg (must be <2deg delta between track and dirto crs)
  if (dist <= turnDist && std::abs(crsToEnd - track) < 2) {
    //FIXME: DEBUG ENTRY
    Logger::GetInstance() << "LEG changed at " << dist << "/" << turnDist << " remaining from " <<
      (int)currentLegStart << " along " << track << " to " << (int)currentLegEnd << " along " << nextCrs << "\n";

    currentLegStart = currentLegEnd;
    currentLegEnd = (currentLegEnd % 9) + 1;
    indicators.indicator.ALERT = false;
    timeInLeg = 0;
    return;
  }

  timeInLeg += dTime;
}
