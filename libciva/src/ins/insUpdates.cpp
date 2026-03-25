#include "ins/ins.h"

namespace libciva {

#pragma region Static Helpers

static std::pair<double, double> dmeCorrection(const DME &dme, const POSITION &pos, const VarManager &varManager,
                                               const double dmeDist, const double dTime) {

  double dist = -1;

  double distFromStation = dme.position.distanceTo(pos);
  double bearing = dme.position.bearingTo(pos);

  double alt = varManager.sim.planeAltitude;

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

#pragma endregion

#pragma region Private

void INS::updateSimPosDelta() noexcept {
  double simLat = varManager.sim.planeLatitude;
  double simLon = varManager.sim.planeLongitude;
  POSITION simPos = {simLat, simLon};

  if (simPos.isValid()) {
    simPosDelta = currentINSPosition - simPos;
  }
}

void INS::updateCurrentINSPosition(const double dTime) noexcept {
  double simLat = varManager.sim.planeLatitude;
  double simLon = varManager.sim.planeLongitude;

  POSITION simPos = {simLat, simLon};

  if (!simPos.isValid()) return;

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
    std::pair<double, double> unit1Data = {-1, -1};
    std::pair<double, double> unit2Data = {-1, -1};
    DME *unit1DME;
    DME *unit2DME;

    // Get DME corrected distance error and bearing error
    if (unitIndex == 0) {
      if (dmeUpdating) {
        unit1DME = &DMEs[activeDME - 1];
        unit1Data = dmeCorrection(*unit1DME, currentINSPosition, varManager, varManager.sim.navDme1, dTime);
      }
      if (unit2 && unit2->dmeUpdating) {
        unit2DME = &unit2->DMEs[unit2->activeDME - 1];
        unit2Data = dmeCorrection(*unit2DME, unit2->currentINSPosition, varManager, varManager.sim.navDme2, dTime);
      }
    } else if (unitIndex == 1) {
      if (dmeUpdating) {
        unit2DME = &DMEs[activeDME - 1];
        unit2Data = dmeCorrection(*unit2DME, currentINSPosition, varManager, varManager.sim.navDme2, dTime);
      }
      if (unit2 && unit2->dmeUpdating) {
        unit1DME = &unit2->DMEs[unit2->activeDME - 1];
        unit1Data = dmeCorrection(*unit1DME, unit2->currentINSPosition, varManager, varManager.sim.navDme1, dTime);
      }
    } else if (unitIndex == 2) {
      if (unit2 && unit2->dmeUpdating) {
        unit1DME = &unit2->DMEs[unit2->activeDME - 1];
        unit1Data = dmeCorrection(*unit1DME, unit2->currentINSPosition, varManager, varManager.sim.navDme1, dTime);
      }
      if (unit3 && unit3->dmeUpdating) {
        unit2DME = &unit3->DMEs[unit3->activeDME - 1];
        unit2Data = dmeCorrection(*unit2DME, unit3->currentINSPosition, varManager, varManager.sim.navDme2, dTime);
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
    } else if (unitIndex == 2) {
      currentINSPosition = unit2 ? unit2->currentINSPosition : unit3 ? unit3->currentINSPosition : currentINSPosition;
    }

    // AI
    if (timeInMode >= DME_AI_TIME) {
      accuracyIndex = std::max(unit1Data.second >= 0 && unit2Data.second >= 0 ? 0 : 1, accuracyIndex - 1);
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

void INS::updateMetrics(const double dTime) noexcept {
  const POSITION pos = currentNavPosition(dTime);

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

  /* TKE */

  trackAngleError = deltaAngle(desiredTrack, track);

  /* GS */

  groundSpeed = varManager.sim.groundVelocity;
}

void INS::updateNav(const double dTime) noexcept {
  // Skip if not in auto
  if (!autoMode) return;

  // Invalid/slow speed, skip
  if (groundSpeed <= 1) return;

  const POSITION pos = currentNavPosition(dTime);

  double crsToEnd = pos.bearingTo(waypoints[currentLegEnd]);
  double nextCrs = waypoints[currentLegEnd].bearingTo(waypoints[(currentLegEnd % 9) + 1]);
  double dist = pos.distanceTo(waypoints[currentLegEnd]);

  // Not yet flown min leg time
  if (timeInLeg < MIN_LEG_TIME) {
    autoModePassed = waypoints[currentLegEnd].inFront(pos, track);

    timeInLeg += dTime;
    return;
  }

  double turnRadius = (groundSpeed * groundSpeed) / (11.26 * std::tan(MAX_BANK_ANGLE * DEG2RAD));
  double delta = nextCrs - track;
  while (delta < -180.0) delta += 360.0;
  while (delta >= 180.0) delta -= 360.0;

  double turnDist = turnRadius * std::tan((std::abs(delta) * DEG2RAD) / 2.0);
  turnDist /= 6076.1154856;

  // Lead in/out
  turnDist += groundSpeed * 2 * LEAD_CORRECTION;

  // Turn point hit, advance leg (must be <2deg delta between track and dirto crs)
  if (autoModePassed || (dist <= turnDist && std::abs(crsToEnd - track) < 2)) {

#ifndef NDEBUG
    if (autoModePassed) Logger::GetInstance() << "Auto leg switch occurred after waypoint was passed\n";

    Logger::GetInstance() << "Leg changed at " << dist << "/" << turnDist << " remaining from WPT " << (int)currentLegStart
                          << " along " << track << " to WPT " << (int)currentLegEnd << ". Next crs " << nextCrs << " to WPT "
                          << (int)((currentLegEnd % 9) + 1) << "\n";
#endif

    currentLegStart = currentLegEnd;
    currentLegEnd = (currentLegEnd % 9) + 1;
    indicators.indicator.ALERT = false;
    autoModePassed = false;
    timeInLeg = 0;
    return;
  }

  timeInLeg += dTime;
}

#pragma endregion

#pragma region Public

void INS::updatePreMix(const double dTime) noexcept {
  // Oven/Battery
  temperatureBatterySim(dTime);
  if (state > INS_STATE::OFF && (batteryRuntime == 0 || batteryTest == BATTERY_TEST::FAILED)) {
    state = INS_STATE::FAIL;
    indicators.value = 0;
    indicators.indicator.MSU_BAT = true;
    valid = SIGNAL_VALIDITY::INV;
    clearDisplay();
  }

  // Aux data
  if (state < INS_STATE::ATT && state >= INS_STATE::ALIGN &&
      (alignSubmode < ALIGN_SUBMODE::MODE_7 || (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode >= MAX_MODE_7))) {
    // Ground track
    calculateTrack();

    // Out of bound errors
    handleOutOfBounds();
  }
  // Switching to ATT
  if (state < INS_STATE::ATT && modeSelector == MODE_SELECTOR::ATT) {
    state = INS_STATE::ATT;
    timeInMode = 0;
    valid = SIGNAL_VALIDITY::ATT;
    clearDisplay();

    double msuBat = indicators.indicator.MSU_BAT;
    indicators.value = 0;
    indicators.indicator.MSU_BAT = msuBat;

    return;
  }
  // Going to OFF
  if (state > INS_STATE::OFF && modeSelector == MODE_SELECTOR::OFF) {
    if (currentINSPosition.isValid()) {
      config->setLastINSPosition(currentINSPosition);
    }
    config->setLastDMEs(DMEs);

    config->save();

    state = INS_STATE::OFF;
    reset(true);
  }
  // Main state
  switch (state) {
    case INS_STATE::OFF: {
      if (modeSelector != MODE_SELECTOR::OFF) {
        // Upmode
        if (batteryRuntime == 0) return;

        state = INS_STATE::STBY;
        displayPosition = config->getLastINSPosition();
        config->getLastDMEs(DMEs);
        indicators.indicator.INSERT = true;
        dmeMode = DME_MODE::INV;
        // Init error radial and distance
        baseRadialDriftPerSecond = distributionRadial->operator()(*randomGen);
        distanceDriftPerSecond = std::abs(distributionDistance->operator()(*randomGen)) / 3600.0;

        timeInMode = 0;
      }
      break;
    }
    case INS_STATE::STBY: {
      if (modeSelector != MODE_SELECTOR::STBY) {
        // Upmode
        state = INS_STATE::ALIGN;
      }
      break;
    }
    case INS_STATE::ALIGN: {
      align(dTime);

      break;
    }
    case INS_STATE::NAV: {
      if (modeSelector == MODE_SELECTOR::STBY) {
        // Downmode
        state = INS_STATE::STBY;
        valid = SIGNAL_VALIDITY::INV;
        reset(false);
      } else if (modeSelector == MODE_SELECTOR::ALIGN) {
        // Downmode
        state = INS_STATE::ALIGN;
        valid = SIGNAL_VALIDITY::INV;
        alignSubmode = ALIGN_SUBMODE::MODE_9;
        radialScalarAlignTime = MAX_RADIAL_ERROR_SCALAR_ALIGN_TIME;
        initialDistanceError = currentDistanceError = 0;
        indicators.indicator.ALERT = false;
      }

      dmeUpdateChecks(dTime);

      updateCurrentINSPosition(dTime);

      // AI
      if (timeInMode >= TIME_PER_AI) {
        accuracyIndex++;
        timeInMode = 0;
      }

      break;
    }
    case INS_STATE::ATT:
    case INS_STATE::FAIL: {
      break;
    }
  }
}

void INS::updateMix() noexcept {
  if (unit2 && unit3 && activePerformanceIndex == 4 && state == INS_STATE::NAV && unit2->state == INS_STATE::NAV &&
      unit3->state == INS_STATE::NAV) {
    POSITION mix = (currentINSPosition + unit2->currentINSPosition + unit3->currentINSPosition) / 3.0;

    currentTripleMixPosition = mix;
  } else {
    currentTripleMixPosition = {999, 999};
  }
}

void INS::updatePostMix(const double dTime) noexcept {
  // NAV
  if (state == INS_STATE::NAV) {
    alertLamp(dTime);
    updateMetrics(dTime);
    updateNav(dTime);
  }

  // Display
  if (state > INS_STATE::OFF && state < INS_STATE::ATT) {
    updateDisplay(dTime);
  }

  // Exports
  exportVars();

  // Time step
  timeInMode += dTime;
}

void INSContainer::update(const double dTime) const noexcept {
  unit1->updatePreMix(dTime);
  if (unit2) unit2->updatePreMix(dTime);
  if (unit3) unit3->updatePreMix(dTime);

  unit1->updateMix();
  if (unit2) unit2->updateMix();
  if (unit3) unit3->updateMix();

  unit1->updatePostMix(dTime);
  if (unit2) unit2->updatePostMix(dTime);
  if (unit3) unit3->updatePostMix(dTime);
}

#pragma endregion

#pragma region Remote

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

void INS::remoteInsertDME(const DME (&dme)[9]) noexcept {
  if (!remoteActive || !hasADEU) return;

  for (size_t i = 0; i < 9; i++) {
    if (!dme[i].position.isValid()) continue;

    memcpy(&DMEs[i], &dme[i], sizeof(DME));
    if (i == activeDME) {
      activeDME = 0;
      dmeArmed = dmeUpdating = false;
    }
  }
}

void INS::remoteInsertWPT(const POSITION (&wpt)[9]) noexcept {
  if (!remoteActive || !hasADEU) return;

  for (size_t i = 1; i < 10; i++) {
    if (i == currentLegEnd || i == currentLegStart) continue;

    if (wpt[i - 1].isValid()) {
      memcpy(&waypoints[i], &wpt[i - 1], sizeof(POSITION));
    }
  }
}

#pragma endregion

} // namespace libciva