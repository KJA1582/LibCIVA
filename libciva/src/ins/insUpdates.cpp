#include "ins/ins.h"

namespace libciva {

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
  POSITION simPos = {varManager.sim.planeLatitude, varManager.sim.planeLongitude};

  if (!simPos.isValid()) return;

  // Max error at 500 GS, base of 0.5kts
  double speedScalar = std::max(0.001, groundSpeed / DRIFT_GS);
  // Radial gain
  double errorRadial = radialDriftPerSecond * speedScalar;
  initialRadialError += errorRadial * dTime;
  // Positional gain
  double errorInitialDist = distanceDriftPerSecond * speedScalar;
  initialDistanceError += errorInitialDist * dTime;
  // Inital pos update
  initialINSPosition = (simPos + simPosDelta).destination(initialDistanceError, initialRadialError);
  initialINSPosition.bound();

  // If we are in aided and any unit is DME Updating
  if (activePerformanceIndex == PERFORMANCE_INDEX::AIDED) {
    // Defaults for unit1 and 2
    bool single = unitIndex != UNIT_INDEX::UNIT_3 && dmeUpdating;
    bool dual = single && unit2 && unit2->dmeUpdating;

    // Unit 3
    if (unitIndex == UNIT_INDEX::UNIT_3) {
      single = (unit2 && unit2->dmeUpdating) || (unit3 && unit3->dmeUpdating);
      dual = unit2 && unit2->dmeUpdating && unit3 && unit3->dmeUpdating &&
             unit2->DMEs[activeDME - 1].position != unit3->DMEs[activeDME - 1].position;
    } else {
      dual = unit2 && DMEs[activeDME - 1].position != unit2->DMEs[activeDME - 1].position;
    }

    // Dual DME
    if (dual) {
      double newError = std::max(DUAL_DME_MIN_ERROR, currentDistanceError - DME_CORRECTION * dTime);
      if (newError < currentDistanceError) currentDistanceError = newError;
    }
    // Single
    else if (single) {
      double newError = std::max(SINGLE_DME_MIN_ERROR, currentDistanceError - DME_CORRECTION * dTime);
      if (newError < currentDistanceError) currentDistanceError = newError;
    }
    // Aided, but not updating
    else {
      // Radial gain
      currentRadialError += errorRadial * dTime;
      // Positional gain
      double errorDist = distanceDriftPerSecond * speedScalar;
      currentDistanceError += errorDist * dTime;
      // Current pos update
      currentINSPosition = (simPos + simPosDelta).destination(currentDistanceError, currentRadialError);
      currentINSPosition.bound();

      return; // Abort here
    }

    // Update pos
    currentINSPosition = (simPos + simPosDelta).destination(currentDistanceError, currentRadialError);
    currentINSPosition.bound();

    // AI
    if (timeInMode >= DME_AI_TIME) {
      accuracyIndex = (uint8_t)std::max(dual ? 1 : 0, accuracyIndex - 1);
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

  groundSpeed = varManager.sim.groundVelocity + speedDriftPerSecond * std::max(0.0, groundSpeed / DRIFT_GS) * dTime;
}

void INS::updateNav(const double dTime) noexcept {
  // Skip if not in auto
  if (!autoMode) return;

  // Invalid/slow speed, skip
  if (groundSpeed <= 1) return;

  const POSITION pos = currentNavPosition(dTime);

  double nextCrs = waypoints[currentLegEnd].bearingTo(waypoints[(currentLegEnd % 9) + 1]);
  double dist = pos.distanceTo(waypoints[currentLegEnd]);
  double passed = !waypoints[currentLegEnd].inFront(pos, track);

  // Not yet flown min leg time
  if (timeInLeg < MIN_LEG_TIME) {
    autoModePassed = passed;
    timeInLeg += dTime;

    return;
  }

  double turnRadius = (groundSpeed * groundSpeed) / (11.26 * std::tan(MAX_BANK_ANGLE * DEG2RAD));
  double delta = nextCrs - track;
  while (delta < -180.0) delta += 360.0;
  while (delta >= 180.0) delta -= 360.0;

  double turnDist = turnRadius * std::tan((std::fabs(delta) * DEG2RAD) / 2.0);
  turnDist /= 6076.1154856;

  // Lead in/out
  turnDist += groundSpeed * 2 * LEAD_CORRECTION;

  // Delayed from min legt ime, turn point hit, or passed: advance leg
  if (autoModePassed || dist <= turnDist || passed) {

#ifndef NDEBUG
    if (autoModePassed) Logger::GetInstance() << "Min leg time delay\n";
    if (passed) Logger::GetInstance() << "Waypoint was passed\n";

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
  // Failed
  if (state == INS_STATE::FAIL && !externalPower) {
    state = INS_STATE::FAIL;
    indicators.value = 0;
    indicators.indicator.MSU_BAT = true;
    valid = SIGNAL_VALIDITY::INV;
    clearDisplay();

    return;
  }

  // Oven/Battery
  temperatureBatterySim(dTime);
  if (state > INS_STATE::OFF && (batteryRuntime <= 1 || batteryTest == BATTERY_TEST::FAILED)) {
    state = INS_STATE::FAIL;
    indicators.value = 0;
    indicators.indicator.MSU_BAT = true;
    valid = SIGNAL_VALIDITY::INV;
    clearDisplay();

#ifndef NDEBUG
    Logger::GetInstance() << "Battery ran out\n";
#endif
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

    bool msuBat = indicators.indicator.MSU_BAT;
    indicators.value = 0;
    indicators.indicator.MSU_BAT = msuBat;
    activePerformanceIndex = PERFORMANCE_INDEX::UNAIDED;

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
        baseRadialDriftPerSecond = (*distributionRadialDrift)(*randomGen);
        distanceDriftPerSecond = std::fabs((*distributionDistanceDrift)(*randomGen)) / 3600.0;
        speedDriftPerSecond = (0.05 + std::fabs((*distributionSpeedDrift)(*randomGen))) / 3600.0;

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
        activePerformanceIndex = PERFORMANCE_INDEX::UNAIDED;
        reset(false);
      } else if (modeSelector == MODE_SELECTOR::ALIGN) {
        // Downmode
        state = INS_STATE::ALIGN;
        valid = SIGNAL_VALIDITY::INV;
        alignSubmode = ALIGN_SUBMODE::MODE_9;
        activePerformanceIndex = PERFORMANCE_INDEX::UNAIDED;
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
  if (unit2 && unit3 && activePerformanceIndex == PERFORMANCE_INDEX::AIDED && state == INS_STATE::NAV &&
      unit2->state == INS_STATE::NAV && unit3->state == INS_STATE::NAV) {
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
    if (displayTimer >= 0.5) {
      updateDisplay(dTime);
      displayTimer = 0;
    } else {
      displayTimer += dTime;
    }
  }

  // Exports
  exportVars();

  // Time step
  timeInMode += dTime;
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