#include "varManager.h"

static uint8_t convertCharacter(uint8_t c) {
  if (c == 10) return 'R';
  if (c == 11) return 'L';
  if (c == 12) return ' ';

  return 48 + c;
}

WinVarManager::WinVarManager() noexcept : libciva::VarManager() {
  setVar(libciva::SIM_VAR_AMBIENT_TEMPERATURE, 15);
  setVar(libciva::SIM_VAR_AMBIENT_WIND_DIRECTION, 284);
  setVar(libciva::SIM_VAR_AMBIENT_WIND_VELOCITY, 36);
  setVar(libciva::SIM_VAR_AIRSPEED_TRUE, 500);
  setVar(libciva::SIM_VAR_GROUND_VELOCITY, 500);
  setVar(libciva::SIM_VAR_PLANE_HEADING_DEGREES_TRUE, 249.2);
  setVar(libciva::SIM_VAR_PLANE_LATITUDE, 50);
  setVar(libciva::SIM_VAR_PLANE_LONGITUDE, 8);
  setVar(libciva::SIM_VAR_NAV_DME_1, 38.9); // for station at 50N009E, 1000ft, 112.00
  setVar(libciva::SIM_VAR_NAV_DME_2, 60.2); // for station at 51N008E, 1500ft, 111.00
  setVar(libciva::SIM_VAR_SIMULATION_RATE, 1);
  setVar(libciva::SIM_VAR_PLANE_ALTITUDE, 32000);
}

void WinVarManager::dump() const noexcept {
  for (auto it = store.begin(); it != store.end(); ++it) {
    if (it->first.find(libciva::DISPLAY_VAR) != std::string::npos) {
      const libciva::DISPLAY v = *reinterpret_cast<const libciva::DISPLAY *>(&it->second);
      auto iIt = store.lower_bound(libciva::INDICATORS_VAR); // FIXME: This selects the wrong unit
      if (iIt == store.end()) continue;
      const libciva::INDICATORS i = *reinterpret_cast<const libciva::INDICATORS *>(&iIt->second);

      std::cout << "                         |";
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
      if (i.indicator.FROM_BLINK) {
        std::cout << "\033[5m" << convertCharacter(v.characters.FROM) << "\033[0m";
      } else {
        std::cout << convertCharacter(v.characters.FROM);
      }
      std::cout << " ";
      if (i.indicator.TO_BLINK) {
        std::cout << "\033[5m" << convertCharacter(v.characters.TO) << "\033[0m";
      } else {
        std::cout << convertCharacter(v.characters.TO);
      }
      std::cout << "|";
    } else if (it->first.find(libciva::INDICATORS_VAR) != std::string::npos) {
      const libciva::INDICATORS i = *reinterpret_cast<const libciva::INDICATORS *>(&it->second);

      std::cout << "       ";
      std::cout << (i.indicator.HOLD ? "HOLD|" : "\033[90mHOLD\033[0m|");
      std::cout << (i.indicator.REMOTE ? "\033[93mREMOTE\033[0m|" : "\033[90mREMOTE\033[0m|");
      std::cout << (i.indicator.INSERT ? "INSERT|" : "\033[90mINSERT\033[0m|");
      std::cout << (i.indicator.ALERT ? "\033[93mALERT\033[0m|" : "\033[90mALERT\033[0m|");
      std::cout << (i.indicator.CDU_BAT ? "\033[93mBAT\033[0m|" : "\033[90mBAT\033[0m|");
      std::cout << (i.indicator.WARN ? "\033[91mWARN\033[0m|" : "\033[90mWARN\033[0m|");
      std::cout << (i.indicator.WAYPOINT_CHANGE ? "WPT CHG" : "\033[90mWPT CHG\033[0m") << std::endl;
      std::cout << "               ";
      std::cout << (i.indicator.READY_NAV ? "\033[92mREADY NAV\033[0m|" : "\033[90mREADY NAV\033[0m|");
      std::cout << (i.indicator.MSU_BAT ? "\033[91mBAT\033[0m|" : "\033[90mBAT\033[0m|");
      std::cout << (i.indicator.DME1 ? "\033[92mDME 1\033[0m|" : "\033[90mDME 1\033[0m|");
      std::cout << (i.indicator.DME2 ? "\033[92mDME 2\033[0m" : "\033[90mDME 2\033[0m");
      std::cout << "               ";
    } else if (it->first.find(libciva::MODE_SELECTOR_POS_VAR) != std::string::npos) {
      std::cout << "                                 ";
      std::cout << (it->second == 0 ? "OFF|" : "\033[90mOFF\033[0m|");
      std::cout << (it->second == 1 ? "STBY|" : "\033[90mSTBY\033[0m|");
      std::cout << (it->second == 2 ? "ALIGN|" : "\033[90mALIGN\033[0m|");
      std::cout << (it->second == 3 ? "NAV|" : "\033[90mNAV\033[0m|");
      std::cout << (it->second == 4 ? "ATT" : "\033[90mATT\033[0m");

    } else if (it->first.find(libciva::DATA_SELECTOR_POS_VAR) != std::string::npos) {
      std::cout << (it->second == 0 ? "TK/GS|" : "\033[90mTK/GS\033[0m|");
      std::cout << (it->second == 1 ? "HDG/DA|" : "\033[90mHDG/DA\033[0m|");
      std::cout << (it->second == 2 ? "XTK/TKE|" : "\033[90mXTK/TKE\033[0m|");
      std::cout << (it->second == 3 ? "POS|" : "\033[90mPOS\033[0m|");
      std::cout << (it->second == 4 ? "WAY PT|" : "\033[90mWAY PT\033[0m|");
      std::cout << (it->second == 5 ? "DIS/TIME|" : "\033[90mDIS/TIME\033[0m|");
      std::cout << (it->second == 6 ? "WIND|" : "\033[90mWIND\033[0m|");
      std::cout << (it->second == 7 ? "DSRTK/STS" : "\033[90mDSRTK/STS\033[0m");
    } else {
      std::cout << std::right << std::setfill(' ') << std::setw(55) << it->second;
    }

    std::cout << " = " << it->first << std::endl;
  }
}
