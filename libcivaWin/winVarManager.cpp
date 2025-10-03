#include "winVarManager.h"

static char convertCharacter(char c) {
  if (c == 10) return 'R';
  if (c == 11) return 'L';
  if (c == 12) return ' ';

  return 48 + c;
}

WinVarManager::WinVarManager() noexcept : VarManager("") {
  setVar(SIM_VAR_AMBIENT_TEMPERATURE, 15);
  setVar(SIM_VAR_AMBIENT_WIND_DIRECTION, 284);
  setVar(SIM_VAR_AMBIENT_WIND_VELOCITY, 36);
  setVar(SIM_VAR_AIRSPEED_TRUE, 190);
  setVar(SIM_VAR_GROUND_VELOCITY, 159.4);
  setVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, 249.2);
  // Expected Drift Angle: 7 left, so HDG > TK (left -> right)
}

void WinVarManager::dump() const noexcept {
  for (auto it = store.begin(); it != store.end(); ++it) {
    double value = it->second;
    if (it->first.find(DISPLAY_VAR) == 0) {
      const DISPLAY v = *reinterpret_cast<const DISPLAY *>(&it->second);
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
      std::cout << " ";
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
      std::cout << " ";
      std::cout << convertCharacter(v.characters.TO);
      std::cout << convertCharacter(v.characters.FROM);
    }
    else if (it->first.find(INDICATORS_VAR) == 0) {
      const uint64_t v = *reinterpret_cast<const uint64_t *>(&it->second);
      std::cout << std::right << std::setfill(' ') << std::setw(18) << "b" << std::bitset<9>(v);
    }
    else {
      std::cout << std::right << std::setfill(' ') << std::setw(27) << it->second;
    }

    std::cout << " = " << it->first << std::endl;
  }
}
