#include "winVarManager.h"

char convertCharacter(char c) {
  if (c == 10) return 'R';
  if (c == 11) return 'L';

  return 48 + c;
}

void WinVarManager::setVar(const std::string &name, double value) noexcept {
  VarManager::setVar(name, value);
}

bool WinVarManager::getVar(const std::string &name, double &value) const noexcept {
  return VarManager::getVar(name, value);
}

void WinVarManager::dump() const noexcept {
  for (auto it = store.begin(); it != store.end(); ++it) {
    std::cout << std::left << std::setfill(' ') << std::setw(40) << it->first;
    std::cout << " = ";

    double value = it->second;
    if (it->first.find(ACTION_MALFUNCTION_CODE_VAR) == 0) {
      switch ((ACTION_MALFUNCTION_CODE)it->second) {
          case ACTION_MALFUNCTION_CODE::A04_45:
            std::cout << std::right << std::setfill(' ') << std::setw(27) << "04 45";
            break;
          case ACTION_MALFUNCTION_CODE::A04_57:
            std::cout << std::right << std::setfill(' ') << std::setw(27) << "04 47";
            break;
          case ACTION_MALFUNCTION_CODE::A06_41:
            std::cout << std::right << std::setfill(' ') << std::setw(27) << "06 41";
            break;
          case ACTION_MALFUNCTION_CODE::A06_43:
            std::cout << std::right << std::setfill(' ') << std::setw(27) << "06 43";
            break;
          case ACTION_MALFUNCTION_CODE::A06_56:
            std::cout << std::right << std::setfill(' ') << std::setw(27) << "06 56";
            break;
          default:
            std::cout << std::right << std::setfill(' ') << std::setw(27) << "     ";
            break;
      }
    }
    else if (it->first.find(DISPLAY_VAR) == 0) {
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

    std::cout << std::endl;
  }
}
