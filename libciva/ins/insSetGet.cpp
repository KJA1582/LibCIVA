#include "ins.h"

void INS::setINSState(INS_STATE state) const noexcept {
  varManager.setVar(STATE_VAR + id, (double)state);
}
INS_STATE INS::getINSState() const noexcept {
  double state;
  if (varManager.getVar(STATE_VAR + id, state)) return (INS_STATE)state;

  return INS_STATE::INV;
}

void INS::setTemperature(double temperature) const noexcept {
  varManager.setVar(TEMPERATURE_VAR + id, temperature);
}
double INS::getTemperature() const noexcept {
  double temperature;
  varManager.getVar(TEMPERATURE_VAR + id, temperature);
  return temperature;
}

void INS::setOperatingTime(double operatingTime) const noexcept {
  varManager.setVar(OPERATING_TIME_VAR + id, operatingTime);
}
double INS::getOperatingTime() const noexcept {
  double operatingTime;
  varManager.getVar(OPERATING_TIME_VAR + id, operatingTime);
  return operatingTime;
}

void INS::setDataSelectorPos(DATA_SELECTOR_POS pos) const noexcept {
  varManager.setVar(DATA_SELECTOR_POS_VAR + id, (double)pos);

  INDICATORS indicators = getIndicators();
  double insLat = getINSPosLat();
  double insLon = getINSPosLon();
  if (isPosValid(insLat, insLon)) {
    indicators.indicator.INSERT = indicators.indicator.WAYPOINT_CHANGE = false;
    setIndicators(indicators);
    setInsertMode(INSERT_MODE::INV);
  }
}
DATA_SELECTOR_POS INS::getDataSelectorPos() const noexcept {
  double pos;
  if (varManager.getVar(DATA_SELECTOR_POS_VAR + id, pos)) return (DATA_SELECTOR_POS)pos;

  return DATA_SELECTOR_POS::INV;
}

void INS::setModeSelectorPos(MODE_SELECTOR_POS pos) const noexcept {
  varManager.setVar(MODE_SELECTOR_POS_VAR + id, (double)pos);
}
MODE_SELECTOR_POS INS::getModeSelectorPos() const noexcept {
  double pos;
  if (varManager.getVar(MODE_SELECTOR_POS_VAR + id, pos)) return (MODE_SELECTOR_POS)pos;

  return MODE_SELECTOR_POS::INV;
}

void INS::setAlignSubmode(ALIGN_SUBMODE mode) const noexcept {
  varManager.setVar(ALIGN_SUBMODE_VAR + id, (double)mode);
}
ALIGN_SUBMODE INS::getAlignSubmode() const noexcept {
  double mode;
  if (varManager.getVar(ALIGN_SUBMODE_VAR + id, mode)) return (ALIGN_SUBMODE)mode;

  return ALIGN_SUBMODE::INV;
}

void INS::setAccuracyIndex(ACCURACY_INDEX index) const noexcept {
  varManager.setVar(ACCURACY_INDEX_VAR + id, (double)index);
}
ACCURACY_INDEX INS::getAccuracyIndex() const noexcept {
  double index;
  if (varManager.getVar(ACCURACY_INDEX_VAR + id, index)) return (ACCURACY_INDEX)index;

  return ACCURACY_INDEX::INV;
}

void INS::setDisplayPosLat(double lat) const noexcept {
  varManager.setVar(DISPLAY_POS_LAT_VAR + id, lat);
}
double INS::getDisplayPosLat() const noexcept {
  double lat;
  if (varManager.getVar(DISPLAY_POS_LAT_VAR + id, lat)) return lat;

  return 999;
}
void INS::setDisplayPosLon(double lon) const noexcept {
  varManager.setVar(DISPLAY_POS_LON_VAR + id, lon);
}
double INS::getDisplayPosLon() const noexcept {
  double lon;
  if (varManager.getVar(DISPLAY_POS_LON_VAR + id, lon)) return lon;

  return 999;
}

void INS::addActionMalfunctionCode(ACTION_MALFUNCTION_CODE code) noexcept {
  if(std::find(malfs.begin(), malfs.end(), code) == malfs.end()) {
    malfs.push_back(code);
  }
}
void INS::clearActionMalfunctionCodes() noexcept {
  malfs.clear();
}
bool INS::hasActionMalfunctionCode(ACTION_MALFUNCTION_CODE code) const noexcept {
  return std::find(malfs.begin(), malfs.end(), code) != malfs.end();
}
bool INS::hasActionMalfunctionCode() const noexcept {
  return !malfs.empty();
}

void INS::setIndicators(INDICATORS indicators) const noexcept {
  varManager.setVar(INDICATORS_VAR + id, indicators.value);
}
INDICATORS INS::getIndicators() const noexcept {
  INDICATORS indicators = { 0 };
  varManager.getVar(INDICATORS_VAR + id, indicators.value);

  return indicators;
}

void INS::setBatteryTestState(BATTERY_TEST state) const noexcept {
  varManager.setVar(BATTERY_TEST_VAR + id, (double)state);
}
BATTERY_TEST INS::getBatteryTestState() const noexcept {
  double state;
  if (varManager.getVar(BATTERY_TEST_VAR + id, state)) return (BATTERY_TEST)state;

  return BATTERY_TEST::IDLE;
}

void INS::setDisplay(DISPLAY display) const noexcept {
  varManager.setVar(DISPLAY_VAR + id, display.value);
}
DISPLAY INS::getDisplay() const noexcept {
  DISPLAY display = { 0 };
  varManager.getVar(DISPLAY_VAR + id, display.value);

  return display;
}

void INS::setInsertMode(INSERT_MODE mode) const noexcept {
  varManager.setVar(INSERT_MODE_VAR + id, (double)mode);
}
INSERT_MODE INS::getInsertMode() const noexcept {
  double mode;
  if (varManager.getVar(INSERT_MODE_VAR + id, mode)) return (INSERT_MODE)mode;

  return INSERT_MODE::INV;
}

void INS::setINSPosLat(double lat) const noexcept {
  varManager.setVar(INS_POS_LAT_VAR + id, lat);
}
double INS::getINSPosLat() const noexcept {
  double lat;
  if (varManager.getVar(INS_POS_LAT_VAR + id, lat)) return lat;

  return 999;
}
void INS::setINSPosLon(double lon) const noexcept {
  varManager.setVar(INS_POS_LON_VAR + id, lon);
}
double INS::getINSPosLon() const noexcept {
  double lon;
  if (varManager.getVar(INS_POS_LON_VAR + id, lon)) return lon;

  return 999;
}

void INS::setWPTSelectorPos(WPT_SELECTOR_POS pos) const noexcept {
  varManager.setVar(WPT_SELECTOR_POS_VAR + id, (double)pos);
}
WPT_SELECTOR_POS INS::getWPTSelectorPos() const noexcept {
  double pos;
  if (varManager.getVar(WPT_SELECTOR_POS_VAR + id, pos)) return (WPT_SELECTOR_POS)pos;

  return WPT_SELECTOR_POS::INV;
}

void INS::setWPTPosLat(double lat, WPT_SELECTOR_POS wpt) const noexcept {
  varManager.setVar(WPT_POS_LAT_VAR + std::to_string((int)wpt) + "_" + id, lat);
}
double INS::getWPTPosLat(WPT_SELECTOR_POS wpt) const noexcept {
  double lon;
  if (varManager.getVar(WPT_POS_LAT_VAR + std::to_string((int)wpt) + "_" + id, lon)) return lon;

  return 999;
}
void INS::setWPTPosLon(double lon, WPT_SELECTOR_POS wpt) const noexcept {
  varManager.setVar(WPT_POS_LON_VAR + std::to_string((int)wpt) + "_" + id, lon);
}
double INS::getWPTPosLon(WPT_SELECTOR_POS wpt) const noexcept {
  double lon;
  if (varManager.getVar(WPT_POS_LON_VAR + std::to_string((int)wpt) + "_" + id, lon)) return lon;

  return 999;
}

void INS::setTrack(double track) const noexcept {
  varManager.setVar(TRACK_VAR + id, track);
}
double INS::getTrack() const noexcept {
  double track;
  if (varManager.getVar(TRACK_VAR + id, track)) return track;

  return 0;
}