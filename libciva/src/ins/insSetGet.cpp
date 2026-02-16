#include "ins/ins.h"

static std::pair<double, double> dmeCorrection(const DME &dme, const POSITION &pos, const VarManager &varManager,
                                               const std::string &dmeID, const double dTime) {

  double dist = -1;

  double distFromStation = dme.position.distanceTo(pos);
  double bearing = dme.position.bearingTo(pos);

  double alt = dme.altitude;
  varManager.getVar(SIM_VAR_PLANE_ALTITUDE, alt);

  double dmeDist = 0;
  bool valid = varManager.getVar(dmeID, dmeDist);
  double slantCorrectedDMEDist = std::sqrt(std::pow(dmeDist, 2) - std::pow((alt - dme.altitude) * 0.000164579, 2));

  double distDelta = slantCorrectedDMEDist - distFromStation;

  // GC dist is more than received dist
  if (distDelta < -0.001) {
    dist = distFromStation - DME_CORRECTION * dTime;
  }
  // GC dist is less than received dist
  else if (distDelta > 0.001) {
    dist = distFromStation + DME_CORRECTION * dTime;
  }

  return std::pair<double, double>(dist, bearing);
}

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

void INS::updateSimPosDelta() noexcept {
  double simLat = 999;
  double simLon = 999;
  varManager.getVar(SIM_VAR_PLANE_LATITUDE, simLat);
  varManager.getVar(SIM_VAR_PLANE_LONGITUDE, simLon);
  POSITION simPos = {simLat, simLon};

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

  POSITION simPos = {simLat, simLon};

  if (!simPos.isValid() || !varManager.getVar(SIM_VAR_GROUND_VELOCITY, groundSpeed)) return;

  // Max error at 500 GS
  double speedScalar = std::max(0.1, groundSpeed / DRIFT_GS);
  // Radial gain
  double errorRadial = radialDriftPerSecond * speedScalar;
  initialRadialError += errorRadial * dTime;
  // Positional gain
  double errorInitialDist = distanceDriftPerSecond * speedScalar;
  initialDistanceError += errorInitialDist * dTime;
  // Inital pos update
  initialINSPosition = (simPos + simPosDelta).destination(initialDistanceError, initialRadialError);
  initialINSPosition.bound();

  if (activePerformanceIndex == 4 && (dmeUpdating || (unit2 && unit2->dmeUpdating) || (unit3 && unit3->dmeUpdating))) {
    std::pair<double, double> unit1Data = {-1, 0};
    std::pair<double, double> unit2Data = {-1, 0};
    DME *unit1DME;
    DME *unit2DME;

    // Get DME corrected distance error and bearing error
    if (id == ID_UNIT_1) {
      if (dmeUpdating) {
        unit1DME = &DMEs[activeDME - 1];
        unit1Data = dmeCorrection(*unit1DME, currentINSPosition, varManager, SIM_VAR_NAV_DME_1, dTime);
      }
      if (unit2 && unit2->dmeUpdating) {
        unit2DME = &unit2->DMEs[unit2->activeDME - 1];
        unit2Data = dmeCorrection(*unit2DME, unit2->currentINSPosition, varManager, SIM_VAR_NAV_DME_2, dTime);
      }
    } else if (id == ID_UNIT_2) {
      if (dmeUpdating) {
        unit2DME = &DMEs[activeDME - 1];
        unit2Data = dmeCorrection(*unit2DME, currentINSPosition, varManager, SIM_VAR_NAV_DME_2, dTime);
      }
      if (unit2 && unit2->dmeUpdating) {
        unit1DME = &unit2->DMEs[unit2->activeDME - 1];
        unit1Data = dmeCorrection(*unit1DME, unit2->currentINSPosition, varManager, SIM_VAR_NAV_DME_1, dTime);
      }
    } else if (id == ID_UNIT_3) {
      if (unit2 && unit2->dmeUpdating) {
        unit1DME = &unit2->DMEs[unit2->activeDME - 1];
        unit1Data = dmeCorrection(*unit1DME, unit2->currentINSPosition, varManager, SIM_VAR_NAV_DME_1, dTime);
      }
      if (unit3 && unit3->dmeUpdating) {
        unit2DME = &unit3->DMEs[unit3->activeDME - 1];
        unit2Data = dmeCorrection(*unit2DME, unit3->currentINSPosition, varManager, SIM_VAR_NAV_DME_2, dTime);
      }
    }

    // Calculate DME corrected position
    POSITION newPosUnit1 = {999, 999};
    POSITION newPosUnit2 = {999, 999};
    POSITION newPos = {999, 999};
    if (unit1Data.first > 0) {
      newPosUnit1 = unit1DME->position.destination(unit1Data.first, unit1Data.second);
    }
    if (unit2Data.first > 0) {
      newPosUnit2 = unit2DME->position.destination(unit2Data.first, unit2Data.second);
    }

    // Validate new INS position
    if (newPosUnit1.isValid() && newPosUnit2.isValid()) {
      newPos = (newPosUnit1 + newPosUnit2) / 2.0;
    } else if (newPosUnit1.isValid()) {
      newPos = newPosUnit1;
    } else if (newPosUnit2.isValid()) {
      newPos = newPosUnit2;
    }

    // Set errors based on new INS position
    if (newPos.isValid()) {
      currentDistanceError = (simPos + simPosDelta).distanceTo(newPos);
      currentRadialError = (simPos + simPosDelta).bearingTo(newPos);
      currentINSPosition = newPos;
      // Since unit 3 cannot update if others are "done", use unit2 or 2 position
    } else if (id == ID_UNIT_3) {
      currentINSPosition = unit2 ? unit2->currentINSPosition : unit3 ? unit3->currentINSPosition : currentINSPosition;
    }

    // AI
    if (timeInMode >= DME_AI_TIME) {
      accuracyIndex = std::max(newPosUnit1.isValid() && newPosUnit2.isValid() ? 0 : 1, accuracyIndex - 1);
      timeInMode = 0;
    }
  } else {
    // Radial gain
    currentRadialError += errorRadial * dTime;
    // Positional gain
    double errorDist = distanceDriftPerSecond * speedScalar;
    currentDistanceError += errorDist * dTime;
    // Current pos update
    currentINSPosition = (simPos + simPosDelta).destination(currentDistanceError, currentRadialError);
    currentINSPosition.bound();
  }
}

void INS::updateMetrics(POSITION &pos) noexcept {
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
  } else {
    _track = (uint16_t)(std::round(track));
  }

  double alongDist = pos.alongTrackDistance(waypoints[currentLegStart], waypoints[currentLegEnd]);
  double legCrs = waypoints[currentLegStart].bearingTo(waypoints[currentLegEnd]);
  POSITION alongPos = waypoints[currentLegStart].destination(alongDist, legCrs);

  remainingDistance = alongPos.distanceTo(waypoints[currentLegEnd]);

  /* XTK */

  crossTrackError = pos.crossTrackDistance(waypoints[currentLegStart], waypoints[currentLegEnd]);

  /* DSRTK */

  if (alongDist < waypoints[currentLegStart].distanceTo(waypoints[currentLegEnd])) {
    desiredTrack = alongPos.bearingTo(waypoints[currentLegEnd]);
  } else {
    desiredTrack = legCrs;
  }
}

void INS::updateNav(POSITION &pos, const double dTime) noexcept {
  // Skip if not in auto
  if (!autoMode) return;

  double gs;
  bool gsValid = varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs);

  // Invalid/slow speed, skip
  if (!gsValid || gs <= 1) return;

  double crsToEnd = pos.bearingTo(waypoints[currentLegEnd]);
  double nextCrs = waypoints[currentLegEnd].bearingTo(waypoints[(currentLegEnd % 9) + 1]);
  double dist = pos.distanceTo(waypoints[currentLegEnd]);
  double legTime = (waypoints[currentLegStart].distanceTo(waypoints[currentLegEnd]) / gs) * 3600;

  // Leg time less than min and not yet flown min leg time
  if (legTime < MIN_LEG_TIME || timeInLeg < MIN_LEG_TIME) {
    timeInLeg += dTime;
    return;
  }

  double turnRadius = (gs * gs) / (11.26 * std::tan(config->getExpectedBankAngle() * DEG2RAD));
  double delta = nextCrs - track;
  while (delta < -180.0) delta += 360.0;
  while (delta >= 180.0) delta -= 360.0;
  double turnDist = turnRadius * std::tan((std::abs(delta) * DEG2RAD) / 2.0);
  turnDist /= 6076.1154856;

  // Turn point hit, advance leg (must be <2deg delta between track and dirto crs)
  if (dist <= turnDist && std::abs(crsToEnd - track) < 2) {

#ifndef NDEBUG
    Logger::GetInstance() << "Leg changed at " << dist << "/" << turnDist << " remaining from WPT " << (int)currentLegStart
                          << " along " << track << " to WPT " << (int)currentLegEnd << ". Next crs " << nextCrs << "to WPT "
                          << (int)((currentLegEnd % 9) + 1) << "\n";
#endif

    currentLegStart = currentLegEnd;
    currentLegEnd = (currentLegEnd % 9) + 1;
    indicators.indicator.ALERT = false;
    timeInLeg = 0;
    return;
  }

  timeInLeg += dTime;
}

void INS::remoteUpdateDME(const uint8_t dme, const bool resetDMEUpdate) noexcept {
  if (!remoteActive) return;

  DME _dme = DMEs[dme];

  if (unit2 && unit2->remoteActive) {
    unit2->DMEs[dme] = _dme;
    if (resetDMEUpdate && unit2->activeDME == dme) {
      activeDME = 0;
    }
  }
  if (unit3 && unit3->remoteActive) {
    unit3->DMEs[dme] = _dme;
  }
}

void INS::remoteUpdateWPT(const uint8_t wpt) noexcept {
  if (!remoteActive) return;

  POSITION _wpt = waypoints[wpt];

  if (unit2 && unit2->remoteActive) {
    unit2->waypoints[wpt] = _wpt;
  }
  if (unit3 && unit3->remoteActive) {
    unit3->waypoints[wpt] = _wpt;
  }
}

void INS::dmeUpdateChecks(const double dTime) noexcept {
  // Unit 3 has no DME connection so is updating whenever 1 or 2 are updating
  if (id == ID_UNIT_3) {
    dmeUpdating = ((unit2 && unit2->dmeUpdating) || (unit3 && unit3->dmeUpdating));
    return;
  }

  // Not armed, skip
  if (!dmeArmed) return;

  // Count time
  if (dmeArmed || dmeUpdating) {
    timeInDME += dTime;
  }

  // Not min time, skip
  if (timeInDME < MIN_DME_TIME) return;

  // Get DME value
  double dmeDist = -1;
  bool valid = id == ID_UNIT_1 ? varManager.getVar(SIM_VAR_NAV_DME_1, dmeDist)
                               : (id == ID_UNIT_2 ? varManager.getVar(SIM_VAR_NAV_DME_2, dmeDist) : false);

  // Drop out of DME if not valid
  if (!valid || dmeDist < 0) {
    dmeArmed = dmeUpdating = false;
    activeDME = 0;
    if (id == ID_UNIT_1) indicators.indicator.DME1 = false;
    if (id == ID_UNIT_2) indicators.indicator.DME2 = false;
  }

  // Check distance, drop out if not reasonable
  DME dme = DMEs[activeDME - 1];

  double alt = dme.altitude;
  varManager.getVar(SIM_VAR_PLANE_ALTITUDE, alt);

  double gcDist = dme.position.distanceTo((currentINSPosition));
  double slantCorrectedDMEDist = std::sqrt(std::pow(dmeDist, 2) - std::pow((alt - dme.altitude) * 0.000164579, 2));
  if (std::abs(gcDist - slantCorrectedDMEDist) > 0.5) {
    dmeArmed = dmeUpdating = false;
    activeDME = 0;
    if (id == ID_UNIT_1) indicators.indicator.DME1 = false;
    if (id == ID_UNIT_2) indicators.indicator.DME2 = false;
    return;
  }

  if (!dmeUpdating) {
    dmeUpdating = true;
    if (id == ID_UNIT_1) indicators.indicator.DME1 = true;
    if (id == ID_UNIT_2) indicators.indicator.DME2 = true;
    timeInMode = 0;
  }
}
