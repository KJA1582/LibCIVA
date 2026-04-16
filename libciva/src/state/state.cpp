#include "state/state.h"

namespace libciva {

State::State(const std::string &basePath, const std::string &id) noexcept : basePath(basePath), id(id) {
  std::ifstream file(basePath + "/libciva_unit_state_" + id, std::ios::binary);
  if (file) {
    file.read((char *)&lastINSPosition, sizeof(lastINSPosition));
    file.read((char *)&lastDMEs, sizeof(lastDMEs));
    file.close();

    if (!lastINSPosition.isValid()) {
      lastINSPosition = {999, 999};
    }
    for (DME &dme : lastDMEs) {
      if (!dme.position.isValid()) {
        dme = {{0, 0}, 0, 0};
      }
      if (dme.frequency > 0 && (dme.frequency < 10800 || dme.frequency > 13595)) {
        dme.frequency = 0;
      }
      if (dme.altitude > 15) {
        dme.altitude = 0;
      }
    }
  }
}

void State::save() const noexcept {
  std::ofstream file(basePath + "/libciva_unit_state_" + id, std::ios::binary);
  if (file) {
    file.write((const char *)&lastINSPosition, sizeof(lastINSPosition));
    file.write((const char *)&lastDMEs, sizeof(lastDMEs));

    file.close();
  }
}

} // namespace libciva