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
  varManager(varManager), id(id), config(Config(workDir)) {

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

void INS::reset(bool full) const noexcept {
  if (full) {
    setINSState(INS_STATE::OFF);
    setModeSelectorPos(MODE_SELECTOR_POS::OFF);
    setDataSelectorPos(DATA_SELECTOR_POS::POS);
    setActionMalfunctionCode(ACTION_MALFUNCTION_CODE::INV);
    setBatteryTestState(BATTERY_TEST::IDLE);
    setDisplay({ 0 });
    setDisplayPosLat(999);
    setDisplayPosLon(999);
    setINSPosLat(999);
    setINSPosLon(999);
  }

  setAlignSubmode(ALIGN_SUBMODE::MODE_9);
  setAccuracyIndex(ACCURACY_INDEX::AI_9);
  setIndicators({ 0 });
  setOperatingTime(0);
}

void INS::update(double dTime) noexcept {
  double operatingTime = getOperatingTime();
  INS_STATE state = getINSState();
  MODE_SELECTOR_POS mode = getModeSelectorPos();
  INDICATORS indicators = getIndicators();

  // Switching to ATT
  if (state < INS_STATE::ATT && mode == MODE_SELECTOR_POS::ATT) {
    setINSState(INS_STATE::ATT);
    setDisplay({ 0 });

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

        // TODO: Load last pos into display and INS pos
        setDisplayPosLat(config.getLastLat());
        setDisplayPosLon(config.getLastLon());
        setINSPosLat(config.getLastLat());
        setINSPosLon(config.getLastLon());

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

  // Heater
  double ambient = 0;
  if (varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, ambient)) {
    setTemperature(heater(config, getTemperature(), ambient, state > INS_STATE::OFF, dTime));
  }

  // Display
  display();

  // Time step
  operatingTime += dTime;
  setOperatingTime(operatingTime);
}
