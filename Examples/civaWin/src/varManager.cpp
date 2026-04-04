#include "varManager.h"

static uint8_t convertCharacter(uint8_t c) {
  if (c == 10) return 'R';
  if (c == 11) return 'L';
  if (c == 12) return ' ';

  return 48 + c;
}

WinVarManager::WinVarManager() noexcept {
  sim.airspeedTrue = 500;
  sim.groundVelocity = 500;
  sim.ambientTemperature = 15;
  sim.ambientWindDirection = 284;
  sim.ambientWindVelocity = 36;
  sim.planeHeadingDegreesTrue = 249.2;
  sim.planeLatitude = 50;
  sim.planeLongitude = 8;
  sim.navDme1 = 38.9; // for station at 50N009E, 1000ft, 112.00
  sim.navDme2 = 60.2; // for station at 51N008E, 1500ft, 111.00
  sim.simulationRate = 1;
  sim.planeAltitude = 32000;

  // Pure AP Demo
  rollRate = 0;
  bankAngle = 0;
  pitchRate = 0;
  pitchAngle = 0;
}

#include <sstream>

static std::string getUnitDisplayLine(const libciva::DISPLAY &v, const libciva::INDICATORS &ind) {
  std::ostringstream oss;
  oss << convertCharacter(v.characters.LEFT_1);
  oss << convertCharacter(v.characters.LEFT_2);
  oss << (v.characters.LEFT_DEG_1 ? "'" : " ");
  oss << convertCharacter(v.characters.LEFT_3);
  oss << (v.characters.LEFT_DEC_1 ? "." : " ");
  oss << convertCharacter(v.characters.LEFT_4);
  oss << (v.characters.LEFT_DEC_2 ? "." : " ");
  oss << convertCharacter(v.characters.LEFT_5);
  oss << (v.characters.LEFT_DEG_2 ? "'" : " ");
  oss << (v.characters.N ? "N" : " ");
  oss << (v.characters.S ? "S" : " ");
  oss << "|";
  oss << convertCharacter(v.characters.RIGHT_1);
  oss << convertCharacter(v.characters.RIGHT_2);
  oss << convertCharacter(v.characters.RIGHT_3);
  oss << (v.characters.RIGHT_DEG_1 ? "'" : " ");
  oss << convertCharacter(v.characters.RIGHT_4);
  oss << (v.characters.RIGHT_DEC_1 ? "." : " ");
  oss << convertCharacter(v.characters.RIGHT_5);
  oss << (v.characters.RIGHT_DEC_2 ? "." : " ");
  oss << convertCharacter(v.characters.RIGHT_6);
  oss << (v.characters.RIGHT_DEG_2 ? "'" : " ");
  oss << (v.characters.E ? "E" : " ");
  oss << (v.characters.W ? "W" : " ");
  oss << "|";
  if (ind.indicator.FROM_BLINK) {
    oss << "\033[5m" << convertCharacter(v.characters.FROM) << "\033[0m";
  } else {
    oss << convertCharacter(v.characters.FROM);
  }
  oss << " ";
  if (ind.indicator.TO_BLINK) {
    oss << "\033[5m" << convertCharacter(v.characters.TO) << "\033[0m";
  } else {
    oss << convertCharacter(v.characters.TO);
  }
  oss << "|";
  return oss.str();
}

static std::string getUnitIndicatorsLine(const libciva::INDICATORS &ind) {
  std::ostringstream oss;
  oss << (ind.indicator.HOLD ? "|\033[97mHOLD\033[0m|" : "|\033[90mHOLD\033[0m|");
  oss << (ind.indicator.REMOTE ? "\033[93mREMOTE\033[0m|" : "\033[90mREMOTE\033[0m|");
  oss << (ind.indicator.INSERT ? "\033[97mINSERT\033[0m|" : "\033[90mINSERT\033[0m|");
  oss << (ind.indicator.ALERT ? "\033[93mALERT\033[0m|" : "\033[90mALERT\033[0m|");
  oss << (ind.indicator.CDU_BAT ? "\033[93mBAT\033[0m|" : "\033[90mBAT\033[0m|");
  oss << (ind.indicator.WARN ? "\033[91mWARN\033[0m|" : "\033[90mWARN\033[0m|");
  oss << (ind.indicator.WAYPOINT_CHANGE ? "\033[97mWPT CHG\033[0m|" : "\033[90mWPT CHG\033[0m|");
  return oss.str();
}

static std::string getUnitIndicatorsLine2(const libciva::INDICATORS &ind) {
  std::ostringstream oss;
  oss << (ind.indicator.READY_NAV ? "|\033[92mREADY NAV\033[0m|" : "|\033[90mREADY NAV\033[0m|");
  oss << (ind.indicator.MSU_BAT ? "\033[91mBAT\033[0m|" : "\033[90mBAT\033[0m|");
  oss << (ind.indicator.DME1 ? "\033[92mDME 1\033[0m|" : "\033[90mDME 1\033[0m|");
  oss << (ind.indicator.DME2 ? "\033[92mDME 2\033[0m|" : "\033[90mDME 2\033[0m|");
  return oss.str();
}

static std::string getModeLine(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << (ins.modeSelectorPos == 0 ? "|OFF|" : "|\033[90mOFF\033[0m|");
  oss << (ins.modeSelectorPos == 1 ? "STBY|" : "\033[90mSTBY\033[0m|");
  oss << (ins.modeSelectorPos == 2 ? "ALIGN|" : "\033[90mALIGN\033[0m|");
  oss << (ins.modeSelectorPos == 3 ? "NAV|" : "\033[90mNAV\033[0m|");
  oss << (ins.modeSelectorPos == 4 ? "ATT|" : "\033[90mATT\033[0m|");
  return oss.str();
}

static std::string getDataLine(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << (ins.dataSelectorPos == 0 ? "|TK/GS|" : "|\033[90mTK/GS\033[0m|");
  oss << (ins.dataSelectorPos == 1 ? "HDG/DA|" : "\033[90mHDG/DA\033[0m|");
  oss << (ins.dataSelectorPos == 2 ? "XTK/TKE|" : "\033[90mXTK/TKE\033[0m|");
  oss << (ins.dataSelectorPos == 3 ? "POS|" : "\033[90mPOS\033[0m|");
  oss << (ins.dataSelectorPos == 4 ? "WAY PT|" : "\033[90mWAY PT\033[0m|");
  oss << (ins.dataSelectorPos == 5 ? "DIS/TIME|" : "\033[90mDIS/TIME\033[0m|");
  oss << (ins.dataSelectorPos == 6 ? "WIND|" : "\033[90mWIND\033[0m|");
  oss << (ins.dataSelectorPos == 7 ? "DSRTK/STS|" : "\033[90mDSRTK/STS\033[0m|");
  return oss.str();
}

static std::string getValuesLine(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "XTK     : " << std::right << std::setfill(' ') << std::setw(13) << ins.crossTrackError;
  return oss.str();
}

static std::string getValuesLine2(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "DTK     : " << std::right << std::setfill(' ') << std::setw(13) << ins.desiredTrack;
  return oss.str();
}

static std::string getValuesLine3(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "TRK     : " << std::right << std::setfill(' ') << std::setw(13) << ins.track;
  return oss.str();
}

static std::string getValuesLine4(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "TKE     : " << std::right << std::setfill(' ') << std::setw(13) << ins.trackAngleError;
  return oss.str();
}

static std::string getValuesLine5(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "DIS     : " << std::right << std::setfill(' ') << std::setw(13) << ins.distance;
  return oss.str();
}

static std::string getValuesLine6(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "GS      : " << std::right << std::setfill(' ') << std::setw(13) << ins.gs;
  return oss.str();
}

static std::string getPowerLine(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "Valid   : " << std::right << std::setfill(' ') << std::setw(13) << ins.powerState ? "Powered" : "Unpowered";
  return oss.str();
}

static std::string getValidLine(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "Valid   : " << std::right << std::setfill(' ') << std::setw(13)
      << (ins.valid == (double)libciva::SIGNAL_VALIDITY::INV   ? "Invalid"
          : ins.valid == (double)libciva::SIGNAL_VALIDITY::NAV ? "Navigation"
                                                               : "Attitude only");
  return oss.str();
}

static std::string getAutoManLine(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "Auto/Man: " << std::right << std::setfill(' ') << std::setw(13) << (ins.autoMode ? "Auto" : "Manual");
  return oss.str();
}

static std::string getWptSelLine(const libciva::VarManager::UnitExport &ins) {
  std::ostringstream oss;
  oss << "Wpt Sel : " << std::right << std::setfill(' ') << std::setw(13) << (int)ins.waypointSelectorPos;
  return oss.str();
}

void WinVarManager::dump() const noexcept {
  std::string lines[3][15];

  for (int i = 0; i < 3; i++) {
    uint64_t combinedDisplay = ((uint64_t)unit[i].displayLeft | (((uint64_t)unit[i].displayRight) << 32));
    const libciva::DISPLAY v = *reinterpret_cast<const libciva::DISPLAY *>(&combinedDisplay);
    const libciva::INDICATORS ind = *reinterpret_cast<const libciva::INDICATORS *>(&unit[i].indicators);

    lines[i][0] = getUnitDisplayLine(v, ind);
    lines[i][1] = getUnitIndicatorsLine(ind);
    lines[i][2] = getUnitIndicatorsLine2(ind);
    lines[i][3] = getModeLine(unit[i]);
    lines[i][4] = getDataLine(unit[i]);
    lines[i][5] = getValuesLine(unit[i]);
    lines[i][6] = getValuesLine2(unit[i]);
    lines[i][7] = getValuesLine3(unit[i]);
    lines[i][8] = getValuesLine4(unit[i]);
    lines[i][9] = getValuesLine5(unit[i]);
    lines[i][10] = getValuesLine6(unit[i]);
    lines[i][11] = getAutoManLine(unit[i]);
    lines[i][12] = getWptSelLine(unit[i]);
    lines[i][13] = getValidLine(unit[i]);
    lines[i][14] = getPowerLine(unit[i]);
  }

  for (int l = 0; l < 15; l++) {
    for (int i = 0; i < 3; i++) {
      const libciva::INDICATORS ind = *reinterpret_cast<const libciva::INDICATORS *>(&unit[i].indicators);

      int padding = 93;
      if (l == 1) padding = 120;
      if (l == 0 || l > 4) padding = 57;
      if (l == 0 && ind.indicator.TO_BLINK) padding = 65;

      std::cout << std::left << std::setfill(' ') << std::setw(padding) << lines[i][l];
      if (i < 2) std::cout << " | ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Sim vars" << std::endl;
  std::cout << "  Airspeed True  : " << std::right << std::setfill(' ') << std::setw(13) << sim.airspeedTrue << std::endl;
  std::cout << "  Ground Velocity: " << std::right << std::setfill(' ') << std::setw(13) << sim.groundVelocity << std::endl;
  std::cout << "  Ambient Temp   : " << std::right << std::setfill(' ') << std::setw(13) << sim.ambientTemperature << std::endl;
  std::cout << "  Wind Direction : " << std::right << std::setfill(' ') << std::setw(13) << sim.ambientWindDirection << std::endl;
  std::cout << "  Wind Velocity  : " << std::right << std::setfill(' ') << std::setw(13) << sim.ambientWindVelocity << std::endl;
  std::cout << "  Heading True   : " << std::right << std::setfill(' ') << std::setw(13) << sim.planeHeadingDegreesTrue
            << std::endl;
  std::cout << "  Latitude       : " << std::right << std::setfill(' ') << std::setw(13) << sim.planeLatitude << std::endl;
  std::cout << "  Longitude      : " << std::right << std::setfill(' ') << std::setw(13) << sim.planeLongitude << std::endl;
  std::cout << "  NAV DME 1      : " << std::right << std::setfill(' ') << std::setw(13) << sim.navDme1 << std::endl;
  std::cout << "  NAV DME 2      : " << std::right << std::setfill(' ') << std::setw(13) << sim.navDme2 << std::endl;
  std::cout << "  Simulation Rate: " << std::right << std::setfill(' ') << std::setw(13) << sim.simulationRate << std::endl;
  std::cout << "  Plane Altitude : " << std::right << std::setfill(' ') << std::setw(13) << sim.planeAltitude << std::endl;

  // Pure AP Demo
  std::cout << "Pure AP Demo" << std::endl;
  std::cout << "  Roll Rate      : " << std::right << std::setfill(' ') << std::setw(13) << rollRate << std::endl;
  std::cout << "  Bank Angle     : " << std::right << std::setfill(' ') << std::setw(13) << bankAngle << std::endl;
  std::cout << "  Pitch Rate     : " << std::right << std::setfill(' ') << std::setw(13) << pitchRate << std::endl;
  std::cout << "  Pitch Angle    : " << std::right << std::setfill(' ') << std::setw(13) << pitchAngle << std::endl;
}
