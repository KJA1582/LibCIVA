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

  if (dmeUpdating) {
    // TODO: DME Update error and time adjustments
    // 1. Get Distance from station to currentINS
    // 2. Get Bearing from station to currentINS
    // 3. Compare distance from DME (slant correct using station alt and our alt) and from 1.
    // 4. If DME < 1., dec. 1. by SCALAR, epsilon of 0.1
    //    If DME > 1., inc. 1. by SCALAR, epsilon of 0.1
    // 5. Calculate new position by using 2. and 4.
    //    For dual DME: Steps 1-4 with Unit2 data, use Unit1 2. and 4. and Unit2 2. and 4. to calculate new position as
    //    intersection
    //    For Unit3: use Unit1 and Unit2 data
    // 6. Calculate distance/bearing between (simPos + simPosDelta) and 5.
    // 7. Save 6. as currentDistanceError/currentRadialError
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
  double alt = DMEs[activeDME - 1].altitude;
  varManager.getVar(SIM_VAR_PLANE_ALTITUDE, alt);

  double gcDist =
      DMEs[activeDME - 1].position.distanceTo((currentTrippleMixPosition.isValid() ? currentTrippleMixPosition : currentINSPosition));
  double slantCorrectedDMEDist = std::sqrt(std::pow((alt - DMEs[activeDME - 1].altitude) * 0.000164579, 2) + std::pow(dmeDist, 2));
  if (std::abs(gcDist - slantCorrectedDMEDist) > 0.5) {
    dmeArmed = dmeUpdating = false;
    activeDME = 0;
    if (id == ID_UNIT_1) indicators.indicator.DME1 = false;
    if (id == ID_UNIT_2) indicators.indicator.DME2 = false;
    return;
  }

  dmeUpdating = true;
  if (id == ID_UNIT_1) indicators.indicator.DME1 = true;
  if (id == ID_UNIT_2) indicators.indicator.DME2 = true;
}
