#include "ins.h"

#pragma region Helpers

static double heater(const Config &config, double currentTempC, double ambientTempC,
                     bool shouldHeat, double dTime) {
  if (shouldHeat && currentTempC >= config.getOperatingTempInC()) return currentTempC;
  if (!shouldHeat && currentTempC <= ambientTempC + 0.1)
    return ambientTempC;

  double cooling = 0.02;
  double loss = cooling * (currentTempC - ambientTempC) * (dTime / config.getUnitMass());

  if (!shouldHeat) return currentTempC - loss;

  double energy = config.getHeaterWattage() * config.getHeaterEfficiency() * dTime;
  double dTemp = energy / (config.getUnitMass() * config.getUnitSpecificHeat());

  return currentTempC + dTemp - loss;
}

#pragma endregion

INS::INS(VarManager &varManager, const std::string &id, const std::string &workDir) noexcept :
  varManager(varManager), id(id), config(Config(workDir)), malfs() {

  setModeSelectorPos(MODE_SELECTOR_POS::OFF);
  setDataSelectorPos(DATA_SELECTOR_POS::POS);
  setWPTSelectorPos(WPT_SELECTOR_POS::WPT_0);

  reset(true);

  double temperature;
  varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, temperature);
  setTemperature(temperature);

  setOperatingTime(0);
}

INS::~INS() noexcept {
  double lat = getDisplayPosLat();
  double lon = getDisplayPosLon();
  if (lat != 999 && lon != 999) {
    config.setLastLat(lat);
    config.setLastLon(lon);
  }

  config.save();
}

void INS::reset(bool full) noexcept {
  INDICATORS i = { 0 };
  i.indicator.INSERT = true;
  setIndicators(i);

  if (full) {
    uint64_t d;
    d = 0x00CCCCCCCC0CCCCC;

    setINSState(INS_STATE::OFF);
    setIndicators({ 0 });
    setDisplay(*(reinterpret_cast<DISPLAY *>(&d)));
    setDisplayPosLat(999);
    setDisplayPosLon(999);
    setINSPosLat(999);
    setINSPosLon(999);
    setTrack(0);

    for (int i = (int)WPT_SELECTOR_POS::WPT_0; i <= (int)WPT_SELECTOR_POS::WPT_9; i++) {
      setWPTPosLat(999, (WPT_SELECTOR_POS)i);
      setWPTPosLon(999, (WPT_SELECTOR_POS)i);
    }

    clearActionMalfunctionCodes();
    setInsertMode(INSERT_MODE::INV);
  }

  setBatteryTestState(BATTERY_TEST::IDLE);
  setAlignSubmode(ALIGN_SUBMODE::MODE_9);
  setAccuracyIndex(ACCURACY_INDEX::AI_9);
  setOperatingTime(0);
}

void INS::update(double dTime) noexcept {
  double operatingTime = getOperatingTime();
  INS_STATE state = getINSState();
  MODE_SELECTOR_POS mode = getModeSelectorPos();
  INDICATORS indicators = getIndicators();

  // Heat
  double ambient = 0;
  if (varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, ambient)) {
    setTemperature(heater(config, getTemperature(), ambient, state > INS_STATE::OFF, dTime));
  }

  // Switching to ATT
  if (state < INS_STATE::ATT && mode == MODE_SELECTOR_POS::ATT) {
    setINSState(INS_STATE::ATT);

    uint64_t d;
    d = 0x00CCCCCCCC0CCCCC;
    setDisplay(*(reinterpret_cast<DISPLAY *>(&d)));

    operatingTime = 0;
    setOperatingTime(operatingTime);
    return;
  }

  // Going to OFF
  if (state > INS_STATE::OFF && mode == MODE_SELECTOR_POS::OFF) {
    double lat = getDisplayPosLat();
    double lon = getDisplayPosLon();
    if (lat != 999 && lon != 999) {
      config.setLastLat(lat);
      config.setLastLon(lon);
    }

    setINSState(INS_STATE::OFF);
    reset(true);

    return;
  }

  // Main state
  switch (state) {
    case INS_STATE::INV: {
      // ERROR CASE
      break;
    }
    case INS_STATE::OFF: {
      if (mode != MODE_SELECTOR_POS::OFF) {
        // Upmode
        setINSState(INS_STATE::STBY);

        setDisplayPosLat(config.getLastLat());
        setDisplayPosLon(config.getLastLon());

        indicators.indicator.INSERT = true;
        setIndicators(indicators);

        operatingTime = 0;
        break;
      }

      return;
    }
    case INS_STATE::STBY: {
      if (mode != MODE_SELECTOR_POS::STBY) {
        // Upmode
        setINSState(INS_STATE::ALIGN);
        reset(false);
      }

      break;
    }
    case INS_STATE::ALIGN: {
      operatingTime = align();

      break;
    }
    case INS_STATE::NAV: {
      if (mode == MODE_SELECTOR_POS::STBY) {
        // Downmode
        setINSState(INS_STATE::STBY);
        reset(false);

        setINSPosLat(999);
        setINSPosLon(999);
      }
      else if (mode == MODE_SELECTOR_POS::ALIGN) {
        // Downmode
        setINSState(INS_STATE::ALIGN);
        // FIXME: Unsure if this downmodes to submode 9
        // It does not for 0, no idea what happens if at > 5
        setAlignSubmode(ALIGN_SUBMODE::MODE_9);
      }

      // TODO: NAV flow
      ACCURACY_INDEX accuracyIndex = getAccuracyIndex();
      if (operatingTime >= TIME_PER_AI && accuracyIndex < ACCURACY_INDEX::AI_9) {
        setAccuracyIndex((ACCURACY_INDEX)((int)accuracyIndex + 1));
        operatingTime = 0;
      }

      break;
    }
    case INS_STATE::ATT: {
      break;
    }
  }

  // Display
  if (state > INS_STATE::OFF && state < INS_STATE::ATT) {
    display();
  }

  // Ground track
 calculateTrack();

  // Out of bound errors
  handleOutOfBounds();

  // Time step
  operatingTime += dTime;
  setOperatingTime(operatingTime);
}

void INS::calculateTrack() const noexcept {
  double trueHeading;
  double windDir;
  double windSpeed;
  double tas;
  double gs;

  if (varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading) &&
      varManager.getVar(SIM_VAR_AMBIENT_WIND_DIRECTION, windDir) &&
      varManager.getVar(SIM_VAR_AMBIENT_WIND_VELOCITY, windSpeed) &&
      varManager.getVar(SIM_VAR_AIRSPEED_TRUE, tas) &&
      varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs)) {

    if (gs < MIN_GS) {
      setTrack(trueHeading);

      return;
    }

    double _trueHeading = trueHeading * M_PI / 180;
    double _windDir = fmod(windDir + 180, 360) * M_PI / 180;

    setTrack(fmod(360 + (atan2(tas * sin(_trueHeading) + windSpeed * sin(_windDir),
                               tas * cos(_trueHeading) + windSpeed * cos(_windDir))) * 180 / M_PI, 360));
  }
}

void INS::handleOutOfBounds() noexcept {
  INDICATORS indicators = getIndicators();

  double gs;
  if (!varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs)) {
    gs = 0;
  };
  double trueHeading;
  if (!varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading)) {
    trueHeading = 0;
  }

  if (abs(trueHeading - getTrack()) > MAX_DRIFT_ANGLE) {
    addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A02_42);
    indicators.indicator.WARN = true;
    setIndicators(indicators);
  }
  if (gs > MAX_GS) {
    addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A02_31);
    indicators.indicator.WARN = true;
    setIndicators(indicators);
  }

  setIndicators(indicators);
}