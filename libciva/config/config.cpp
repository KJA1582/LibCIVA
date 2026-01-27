#include "config.h"

Config::Config(const std::string &basePath, const std::string &id) noexcept : basePath(basePath), id(id) {
  std::ifstream file(basePath + "config_" + id, std::ios::binary);
  if (file) {
    file.read((char *)&operatingTempInC, sizeof(operatingTempInC));
    file.read((char *)&heaterWattage, sizeof(heaterWattage));
    file.read((char *)&heaterEfficiency, sizeof(heaterEfficiency));
    file.read((char *)&unitMassInKG, sizeof(unitMassInKG));
    file.read((char *)&unitSpecificHeat, sizeof(unitSpecificHeat));
    file.read((char *)&lastINSPosition, sizeof(lastINSPosition));

    file.read((char *)&lastDMEs, sizeof(lastDMEs));

    file.read((char *)&expectedBankAngle, sizeof(expectedBankAngle));

    file.close();
  }
}

void Config::save() const noexcept {
  std::ofstream file(basePath + "config_" + id, std::ios::binary);
  if (file) {
    file.write((const char *)&operatingTempInC, sizeof(operatingTempInC));
    file.write((const char *)&heaterWattage, sizeof(heaterWattage));
    file.write((const char *)&heaterEfficiency, sizeof(heaterEfficiency));
    file.write((const char *)&unitMassInKG, sizeof(unitMassInKG));
    file.write((const char *)&unitSpecificHeat, sizeof(unitSpecificHeat));
    file.write((const char *)&lastINSPosition, sizeof(lastINSPosition));

    file.write((const char*)&lastDMEs, sizeof(lastDMEs));

    file.write((const char *)&expectedBankAngle, sizeof(expectedBankAngle));

    file.close();
  }
}