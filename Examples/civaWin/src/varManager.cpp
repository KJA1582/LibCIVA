#include "varManager.h"

static uint8_t convertCharacter(uint8_t c) {
  if (c == 10) return 'R';
  if (c == 11) return 'L';
  if (c == 12) return ' ';

  return 48 + c;
}

WinVarManager::WinVarManager() noexcept {
  sim.ambientTemperature = 15;
  sim.ambientWindDirection = 284;
  sim.ambientWindVelocity = 36;
  sim.airspeedTrue = 500;
  sim.groundVelocity = 500;
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
}

void WinVarManager::dump() const noexcept {
  for (int i = 0; i < 1; i++) {
    const libciva::DISPLAY v = *reinterpret_cast<const libciva::DISPLAY *>(&unit[i].display);
    const libciva::INDICATORS ind = *reinterpret_cast<const libciva::INDICATORS *>(&unit[i].indicators);

    std::cout << "Unit " << (i + 1) << ":" << std::endl;
    std::cout << convertCharacter(v.characters.LEFT_1);
    std::cout << convertCharacter(v.characters.LEFT_2);
    std::cout << (v.characters.LEFT_DEG_1 ? "'" : " ");
    std::cout << convertCharacter(v.characters.LEFT_3);
    std::cout << (v.characters.LEFT_DEC_1 ? "." : " ");
    std::cout << convertCharacter(v.characters.LEFT_4);
    std::cout << (v.characters.LEFT_DEC_2 ? "." : " ");
    std::cout << convertCharacter(v.characters.LEFT_5);
    std::cout << (v.characters.LEFT_DEG_2 ? "'" : " ");
    std::cout << (v.characters.N ? "N" : " ");
    std::cout << (v.characters.S ? "S" : " ");
    std::cout << "|";
    std::cout << convertCharacter(v.characters.RIGHT_1);
    std::cout << convertCharacter(v.characters.RIGHT_2);
    std::cout << convertCharacter(v.characters.RIGHT_3);
    std::cout << (v.characters.RIGHT_DEG_1 ? "'" : " ");
    std::cout << convertCharacter(v.characters.RIGHT_4);
    std::cout << (v.characters.RIGHT_DEC_1 ? "." : " ");
    std::cout << convertCharacter(v.characters.RIGHT_5);
    std::cout << (v.characters.RIGHT_DEC_2 ? "." : " ");
    std::cout << convertCharacter(v.characters.RIGHT_6);
    std::cout << (v.characters.RIGHT_DEG_2 ? "'" : " ");
    std::cout << (v.characters.E ? "E" : " ");
    std::cout << (v.characters.W ? "W" : " ");
    std::cout << "|";
    if (ind.indicator.FROM_BLINK) {
      std::cout << "\033[5m" << convertCharacter(v.characters.FROM) << "\033[0m";
    } else {
      std::cout << convertCharacter(v.characters.FROM);
    }
    std::cout << " ";
    if (ind.indicator.TO_BLINK) {
      std::cout << "\033[5m" << convertCharacter(v.characters.TO) << "\033[0m";
    } else {
      std::cout << convertCharacter(v.characters.TO);
    }
    std::cout << "|" << std::endl;

    std::cout << (ind.indicator.HOLD ? "HOLD|" : "\033[90mHOLD\033[0m|");
    std::cout << (ind.indicator.REMOTE ? "\033[93mREMOTE\033[0m|" : "\033[90mREMOTE\033[0m|");
    std::cout << (ind.indicator.INSERT ? "INSERT|" : "\033[90mINSERT\033[0m|");
    std::cout << (ind.indicator.ALERT ? "\033[93mALERT\033[0m|" : "\033[90mALERT\033[0m|");
    std::cout << (ind.indicator.CDU_BAT ? "\033[93mBAT\033[0m|" : "\033[90mBAT\033[0m|");
    std::cout << (ind.indicator.WARN ? "\033[91mWARN\033[0m|" : "\033[90mWARN\033[0m|");
    std::cout << (ind.indicator.WAYPOINT_CHANGE ? "WPT CHG" : "\033[90mWPT CHG\033[0m") << std::endl;
    std::cout << (ind.indicator.READY_NAV ? "\033[92mREADY NAV\033[0m|" : "\033[90mREADY NAV\033[0m|");
    std::cout << (ind.indicator.MSU_BAT ? "\033[91mBAT\033[0m|" : "\033[90mBAT\033[0m|");
    std::cout << (ind.indicator.DME1 ? "\033[92mDME 1\033[0m|" : "\033[90mDME 1\033[0m|");
    std::cout << (ind.indicator.DME2 ? "\033[92mDME 2\033[0m" : "\033[90mDME 2\033[0m") << std::endl;

    std::cout << (unit[i].modeSelectorPos == 0 ? "OFF|" : "\033[90mOFF\033[0m|");
    std::cout << (unit[i].modeSelectorPos == 1 ? "STBY|" : "\033[90mSTBY\033[0m|");
    std::cout << (unit[i].modeSelectorPos == 2 ? "ALIGN|" : "\033[90mALIGN\033[0m|");
    std::cout << (unit[i].modeSelectorPos == 3 ? "NAV|" : "\033[90mNAV\033[0m|");
    std::cout << (unit[i].modeSelectorPos == 4 ? "ATT" : "\033[90mATT\033[0m") << std::endl;

    std::cout << (unit[i].dataSelectorPos == 0 ? "TK/GS|" : "\033[90mTK/GS\033[0m|");
    std::cout << (unit[i].dataSelectorPos == 1 ? "HDG/DA|" : "\033[90mHDG/DA\033[0m|");
    std::cout << (unit[i].dataSelectorPos == 2 ? "XTK/TKE|" : "\033[90mXTK/TKE\033[0m|");
    std::cout << (unit[i].dataSelectorPos == 3 ? "POS|" : "\033[90mPOS\033[0m|");
    std::cout << (unit[i].dataSelectorPos == 4 ? "WAY PT|" : "\033[90mWAY PT\033[0m|");
    std::cout << (unit[i].dataSelectorPos == 5 ? "DIS/TIME|" : "\033[90mDIS/TIME\033[0m|");
    std::cout << (unit[i].dataSelectorPos == 6 ? "WIND|" : "\033[90mWIND\033[0m|");
    std::cout << (unit[i].dataSelectorPos == 7 ? "DSRTK/STS" : "\033[90mDSRTK/STS\033[0m") << std::endl;

    std::cout << "XTK: " << std::right << std::setfill(' ') << std::setw(12) << unit[i].crossTrackError << std::endl;
    std::cout << "DTK: " << std::right << std::setfill(' ') << std::setw(12) << unit[i].desiredTrack << std::endl;
    std::cout << "DIS: " << std::right << std::setfill(' ') << std::setw(12) << unit[i].distance << std::endl;
    std::cout << "Valid: " << (unit[i].valid ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
  }

  std::cout << "Sim vars:" << std::endl;
  std::cout << "  Airspeed True:    " << sim.airspeedTrue << std::endl;
  std::cout << "  Ambient Temp:     " << sim.ambientTemperature << std::endl;
  std::cout << "  Wind Direction:   " << sim.ambientWindDirection << std::endl;
  std::cout << "  Wind Velocity:    " << sim.ambientWindVelocity << std::endl;
  std::cout << "  Ground Velocity:  " << sim.groundVelocity << std::endl;
  std::cout << "  Heading True:     " << sim.planeHeadingDegreesTrue << std::endl;
  std::cout << "  Latitude:         " << sim.planeLatitude << std::endl;
  std::cout << "  Longitude:        " << sim.planeLongitude << std::endl;
  std::cout << "  NAV DME 1:        " << sim.navDme1 << std::endl;
  std::cout << "  NAV DME 2:        " << sim.navDme2 << std::endl;
  std::cout << "  Simulation Rate:  " << sim.simulationRate << std::endl;
  std::cout << "  Plane Altitude:   " << sim.planeAltitude << std::endl;

  // Pure AP Demo
  std::cout << "  Roll Rate:        " << rollRate << std::endl;
  std::cout << "  Bank Angle:       " << bankAngle << std::endl;
}
