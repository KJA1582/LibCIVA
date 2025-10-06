#include "config.h"

Config::Config(const std::string &basePath) noexcept : basePath(basePath) {
  std::ifstream file(basePath + "config", std::ios::binary);
  if (file) {
    file.read((char *)&operatingTempInC, sizeof(operatingTempInC));
    file.read((char *)&heaterWattage, sizeof(heaterWattage));
    file.read((char *)&heaterEfficiency, sizeof(heaterEfficiency));
    file.read((char *)&unitMass, sizeof(unitMass));
    file.read((char *)&unitSpecificHeat, sizeof(unitSpecificHeat));
    file.read((char *)&lastINSPosition, sizeof(lastINSPosition));

    file.read((char *)&lastDMEs, sizeof(lastDMEs));

    file.close();
  }
}

void Config::save() const noexcept {
  std::ofstream file(basePath + "config", std::ios::binary);
  if (file) {
    file.write((const char *)&operatingTempInC, sizeof(operatingTempInC));
    file.write((const char *)&heaterWattage, sizeof(heaterWattage));
    file.write((const char *)&heaterEfficiency, sizeof(heaterEfficiency));
    file.write((const char *)&unitMass, sizeof(unitMass));
    file.write((const char *)&unitSpecificHeat, sizeof(unitSpecificHeat));
    file.write((const char *)&lastINSPosition, sizeof(lastINSPosition));

    file.write((const char*)&lastDMEs, sizeof(lastDMEs));

    file.close();
  }
}