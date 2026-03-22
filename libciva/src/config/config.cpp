#include "config/config.h"

namespace libciva {

Config::Config(const std::string &basePath, const std::string &id) noexcept : basePath(basePath), id(id) {
  std::ifstream file(basePath + "/config_" + id, std::ios::binary);
  if (file) {
    file.read((char *)&lastINSPosition, sizeof(lastINSPosition));

    file.read((char *)&lastDMEs, sizeof(lastDMEs));

    file.read((char *)&expectedBankAngle, sizeof(expectedBankAngle));

    file.close();
  }
}

void Config::save() const noexcept {
  std::ofstream file(basePath + "/config_" + id, std::ios::binary);
  if (file) {
    file.write((const char *)&lastINSPosition, sizeof(lastINSPosition));

    file.write((const char *)&lastDMEs, sizeof(lastDMEs));

    file.write((const char *)&expectedBankAngle, sizeof(expectedBankAngle));

    file.close();
  }
}

} // namespace libciva