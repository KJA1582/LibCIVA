#include "ins.h"

#pragma region Lifecycle

INS::INS(VarManager &varManager, const std::string &id, const std::string &workDir) noexcept :
  varManager(varManager), config(Config(workDir)), id(id), actionMalfunctionCodes() {
  clearDisplay();

  // Init all things from global vars
  varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, ovenTemperature);

  // Init exports
  exportVars();
}

INS::~INS() noexcept {
  // If we have a valid position, save
  if (currentINSPosition.isValid()) {
    config.setLastINSPosition(currentINSPosition);
  }
  config.setLastDMEs(DMEs);

  config.save();
}

#pragma endregion

void INS::temperatureSim(const double dTime) noexcept {
  bool shouldHeat = state > INS_STATE::OFF;

  double ambient = 0;
  if (varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, ambient)) {
    // Exit if at operating tem or ambiant in case of heating or no heating
    if (shouldHeat && ovenTemperature >= config.getOperatingTempInC()) return;
    if (!shouldHeat && ovenTemperature <= ambient + 0.1)  return;

    double cooling = 0.02;
    double loss = cooling * (ovenTemperature - ambient) * (dTime / config.getUnitMass());

    // Pure cooling down
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
    indicators = { 0 };
    displayPosition = currentINSPosition = initialINSPosition = { 999, 999 };
    track = 0;

    for (uint8_t i = 0; i < 10; i++) {
      waypoints[i] = { 999, 999 };
    }

    insertMode = INSERT_MODE::INV;
    displayPerformanceIndex = activePerformanceIndex = 5;
    mafunctionCodeDisplayed = inTestMode = false;
    dmeMode = DME_MODE::INV;
    displayActionMalfunctionCodeIndex = 0;
    clearActionMalfunctionCodes();
  }

  batteryTest = BATTERY_TEST::IDLE;
  alignSubmode = ALIGN_SUBMODE::MODE_9;
  accuracyIndex = 9;
  timeInMode = 0;
  timeInNAV = INTIAL_TIME_IN_NAV;
}

void INS::calculateTrack() noexcept {
  double trueHeading = 0;
  double windDir = 0;
  double windSpeed = 0;
  double tas = 0;
  double gs = 0;

  if (varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading) &&
      varManager.getVar(SIM_VAR_AMBIENT_WIND_DIRECTION, windDir) &&
      varManager.getVar(SIM_VAR_AMBIENT_WIND_VELOCITY, windSpeed) &&
      varManager.getVar(SIM_VAR_AIRSPEED_TRUE, tas) &&
      varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs)) {

    if (gs < MIN_GS) {
      track = trueHeading;

      return;
    }

    double _trueHeading = trueHeading * M_PI / 180;
    double _windDir = std::fmod(windDir + 180, 360) * M_PI / 180;

    track = std::fmod(360 + (std::atan2(tas * std::sin(_trueHeading) + windSpeed * std::sin(_windDir),
                              tas * std::cos(_trueHeading) + windSpeed * std::cos(_windDir))) *
                 180 / M_PI, 360);
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

  if (abs(trueHeading - track) > MAX_DRIFT_ANGLE) {
    addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A02_42);
    indicators.indicator.WARN = true;
  }
  if (gs > MAX_GS) {
    addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A02_31);
    indicators.indicator.WARN = true;
  }

  // TODO: Inter system compare
}

void INS::exportVars() const noexcept {
  varManager.setVar(std::string(VAR_START) + DISPLAY_VAR + id, display.value);
  varManager.setVar(std::string(VAR_START) + INDICATORS_VAR + id, indicators.value);
  varManager.setVar(std::string(VAR_START) + MODE_SELECTOR_POS_VAR + id, (double)modeSelector);
  varManager.setVar(std::string(VAR_START) + DATA_SELECTOR_POS_VAR + id, (double)dataSelector);
  varManager.setVar(std::string(VAR_START) + WAYPOINT_SELECTOR_POS_VAR + id, (double)waypointSelector);
}

void INS::update(const double dTime) noexcept {
  // Oven
  temperatureSim(dTime);
  // Aux data
  if (state >= INS_STATE::ALIGN && (alignSubmode < ALIGN_SUBMODE::MODE_7 ||
                                    (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode >= MAX_MODE_7))) {
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
        displayPosition = config.getLastINSPosisiton();
        config.getLastDMEs(DMEs);
        indicators.indicator.INSERT = true;
        dmeMode = DME_MODE::INV;

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
      }
      else if (modeSelector == MODE_SELECTOR::ALIGN) {
        // Downmode
        state = INS_STATE::ALIGN;
        alignSubmode = ALIGN_SUBMODE::MODE_9;
        timeInNAV = INTIAL_TIME_IN_NAV;
      }

      // TODO: NAV flow

      // AI
      if (timeInMode >= TIME_PER_AI && accuracyIndex < 9) {
        accuracyIndex++;
        timeInMode = 0;
      }

      timeInNAV += dTime;
      break;
    }
    case INS_STATE::ATT:
    case INS_STATE::FAIL: {
      break;
    }
  }



  // Display
  if (state > INS_STATE::OFF && state < INS_STATE::ATT) {
    updateDisplay();
  }

  // Exports
  exportVars();

  // Time step
  timeInMode += dTime;
}
