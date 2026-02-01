#include "ins/ins.h"

#pragma region Lifecycle

std::random_device rd;
std::mt19937 rg(rd());
std::uniform_real_distribution<> disRad(0.0, 360.0);
std::normal_distribution<> disDist(0);

INS::INS(VarManager &varManager, const std::string &id, const std::string &configID, const std::string &workDir,
         const bool hasDME) noexcept
    : varManager(varManager), config(Config(workDir, configID)), id(id), actionMalfunctionCodes(), hasDME(hasDME) {
  clearDisplay();

  // Init all things from global vars
  varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, ovenTemperature);

  errorRadial = disRad(rg);
  // Normal distributed drift in nmi per s
  driftPerSecond = std::abs(disDist(rg)) / 3600.0;

  // Init exports
  exportVars();
}

INS::~INS() noexcept {
  // If we have a valid position, save
  if (currentINSPosition.isValid()) {
    config.setLastINSPosition(currentINSPosition);
  }
  config.setLastDMEs(DMEs);
}

#pragma endregion

void INS::temperatureSim(const double dTime) noexcept {
  bool shouldHeat = state > INS_STATE::OFF;

  double ambient = 0;
  if (varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, ambient)) {
    // Exit if at operating tem or ambient in case of heating or no heating
    if (shouldHeat && ovenTemperature >= config.getOperatingTempInC()) return;

    double cooling = shouldHeat ? 0.0 : 0.02;
    double loss = cooling * (ovenTemperature - ambient) * (dTime / config.getUnitMass());

    // Ambient following
    if (!shouldHeat) {
      ovenTemperature -= loss;
      return;
    }

    double energy = config.getHeaterWattage() * config.getHeaterEfficiency() * dTime;
    double dTemp = energy / (config.getUnitMass() * config.getUnitSpecificHeat());

    ovenTemperature = ovenTemperature + dTemp - loss;
  }
}

void INS::reset(const bool full) noexcept {
  if (full) {
    clearDisplay();
    indicators = {0};
    displayPosition = currentINSPosition = initialINSPosition = {999, 999};
    track = 0;

    for (uint8_t i = 0; i < 10; i++) {
      waypoints[i] = {0, 0};
    }

    insertMode = INSERT_MODE::INV;
    activePerformanceIndex = 5;
    malfunctionCodeDisplayed = inTestMode = false;
    dmeMode = DME_MODE::INV;

    actionMalfunctionCodes.value = 0;
    displayActionMalfunctionCodeIndex = 0;
  }

  batteryTest = BATTERY_TEST::IDLE;
  alignSubmode = ALIGN_SUBMODE::MODE_9;
  accuracyIndex = 9;
  timeInMode = 0;
  initialTimeInNAV = timeInNAV = INITIAL_TIME_IN_NAV;
  initialError = currentError = 0;
  indicators.indicator.READY_NAV = false;
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

void INS::handleOutOfBounds() noexcept {
  double gs;
  if (!varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs)) {
    gs = 0;
  };
  double trueHeading;
  if (!varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading)) {
    trueHeading = 0;
  }

  if (deltaAngle(trueHeading, track) > MAX_DRIFT_ANGLE) {
    actionMalfunctionCodes.codes.A02_42 = true;
    advanceActionMalfunctionIndex();
    indicators.indicator.WARN = true;
  }
  if (gs > MAX_GS) {
    actionMalfunctionCodes.codes.A02_31 = true;
    advanceActionMalfunctionIndex();
    indicators.indicator.WARN = true;
  }

  // Inter system compare trigger 04-43
  if (state == INS_STATE::ALIGN) {
    if (unit2) {
      if (unit2->getINSPosition().distanceTo(initialINSPosition) > 0.1) {
        actionMalfunctionCodes.codes.A04_43 = true;
        advanceActionMalfunctionIndex();
        indicators.indicator.WARN = true;
      }
    }
    if (unit3) {
      if (unit3->getINSPosition().distanceTo(initialINSPosition) > 0.1) {
        actionMalfunctionCodes.codes.A04_43 = true;
        advanceActionMalfunctionIndex();
        indicators.indicator.WARN = true;
      }
    }
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
}

void INS::updatePreMix(const double dTime) noexcept {
  // Oven
  temperatureSim(dTime);
  // Aux data
  if (state >= INS_STATE::ALIGN &&
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
    clearDisplay();

    return;
  }
  // Going to OFF
  if (state > INS_STATE::OFF && modeSelector == MODE_SELECTOR::OFF) {
    if (currentINSPosition.isValid()) {
      config.setLastINSPosition(currentINSPosition);
    }
    config.setLastDMEs(DMEs);

    config.save();

    state = INS_STATE::OFF;
    reset(true);
  }
  // Main state
  switch (state) {
    case INS_STATE::OFF: {
      if (modeSelector != MODE_SELECTOR::OFF) {
        // Upmode
        state = INS_STATE::STBY;
        displayPosition = config.getLastINSPosition();
        config.getLastDMEs(DMEs);
        indicators.indicator.INSERT = true;
        dmeMode = DME_MODE::INV;
        // Init error radial and distance
        errorRadial = disRad(rg);
        driftPerSecond = std::abs(disDist(rg)) / 3600.0;

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
        reset(false);
      } else if (modeSelector == MODE_SELECTOR::ALIGN) {
        // Downmode
        state = INS_STATE::ALIGN;
        alignSubmode = ALIGN_SUBMODE::MODE_9;
        initialTimeInNAV = timeInNAV = INITIAL_TIME_IN_NAV;
        initialError = currentError = 0;
      }

      updateCurrentINSPosition(dTime);

      // AI
      if (timeInMode >= TIME_PER_AI && accuracyIndex < 9) {
        accuracyIndex++;
        timeInMode = 0;
      }

      initialTimeInNAV += dTime;
      timeInNAV += dTime;
      break;
    }
    case INS_STATE::ATT:
    case INS_STATE::FAIL: {
      break;
    }
  }
}

void INS::updateMix() noexcept {
  if (unit2 && unit3 && activePerformanceIndex == 4 && unit2->activePerformanceIndex == 4 && unit3->activePerformanceIndex == 4 &&
      state == INS_STATE::NAV && unit2->state == INS_STATE::NAV && unit3->state == INS_STATE::NAV) {
    // Uses averaging instead of fancy formulas.
    // Circumcenter was tried, but the result was very erratic.
    // Using averages will result in inaccuracy on high flight hours, but at 20h of uncorrected flight, worst case (3sigma) is 20
    // miles. Which is bad to begin with.
    POSITION mix = (currentINSPosition + unit2->currentINSPosition + unit3->currentINSPosition) / 3.0;

    currentTrippleMixPosition = unit2->currentTrippleMixPosition = unit3->currentTrippleMixPosition = mix;
  } else {
    currentTrippleMixPosition = {999, 999};
    if (unit2) unit2->currentTrippleMixPosition = {999, 999};
    if (unit3) unit3->currentTrippleMixPosition = {999, 999};
  }

#ifndef NDEBUG
  double lat;
  double lon;
  varManager.getVar(SIM_VAR_PLANE_LATITUDE, lat);
  varManager.getVar(SIM_VAR_PLANE_LONGITUDE, lon);

  DataLogger::GetInstance() << id << ";" << currentINSPosition << ";" << initialINSPosition << ";" << currentTrippleMixPosition
                            << ";" << lat << "," << lon << "\n";
  if (unit2)
    DataLogger::GetInstance() << unit2->id << ";" << unit2->currentINSPosition << ";" << unit2->initialINSPosition << ";"
                              << unit2->currentTrippleMixPosition << ";" << lat << "," << lon << "\n";
  if (unit3)
    DataLogger::GetInstance() << unit3->id << ";" << unit3->currentINSPosition << ";" << unit3->initialINSPosition << ";"
                              << unit3->currentTrippleMixPosition << ";" << lat << "," << lon << "\n";
#endif
}

void INS::updatePostMix(const double dTime) noexcept {
  // NAV
  if (state == INS_STATE::NAV) {
    alertLamp(currentTrippleMixPosition.isValid() ? currentTrippleMixPosition : currentINSPosition, dTime);
    updateMetrics(currentTrippleMixPosition.isValid() ? currentTrippleMixPosition : currentINSPosition);
    updateNav(currentTrippleMixPosition.isValid() ? currentTrippleMixPosition : currentINSPosition, dTime);
  }

  // Display
  if (state > INS_STATE::OFF && state < INS_STATE::ATT) {
    updateDisplay(currentTrippleMixPosition.isValid() ? currentTrippleMixPosition : currentINSPosition);
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

  // Unit 1 updates *all* via interconnect pointers
  unit1->updateMix();

  unit1->updatePostMix(dTime);
  if (unit2) unit2->updatePostMix(dTime);
  if (unit3) unit3->updatePostMix(dTime);
}
