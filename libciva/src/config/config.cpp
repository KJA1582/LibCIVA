#include "config/config.h"

Config::Config(const std::string &basePath, const std::string &id) noexcept : basePath(basePath), id(id) {
  std::ifstream file(basePath + "/config_" + id, std::ios::binary);
  if (file) {
    file.read((char *)&operatingTempInC, sizeof(operatingTempInC));
    file.read((char *)&heaterWattage, sizeof(heaterWattage));
    file.read((char *)&heaterEfficiency, sizeof(heaterEfficiency));
    file.read((char *)&unitMassInKG, sizeof(unitMassInKG));
    file.read((char *)&unitSpecificHeat, sizeof(unitSpecificHeat));
    file.read((char *)&lastINSPosition, sizeof(lastINSPosition));

    file.read((char *)&lastDMEs, sizeof(lastDMEs));

    file.read((char *)&expectedBankAngle, sizeof(expectedBankAngle));
    file.read((char *)&errorRadial, sizeof(errorRadial));
    file.read((char *)&errorDistance, sizeof(errorDistance));

    file.close();
  }

  std::random_device rd;
  std::mt19937 rg(rd());
  std::uniform_real_distribution<> disRad(0.0, 360.0);
  std::normal_distribution<> disDist(0);
  if (errorRadial < 0) {
    errorRadial = disRad(rg);
  }
  if (errorDistance < 0) {
    // Normal distributed drift in nmi per s
    errorDistance = std::abs(disDist(rg)) / 3600.0;
  }
}

void Config::save() const noexcept {
  std::ofstream file(basePath + "/config_" + id, std::ios::binary);
  if (file) {
    file.write((const char *)&operatingTempInC, sizeof(operatingTempInC));
    file.write((const char *)&heaterWattage, sizeof(heaterWattage));
    file.write((const char *)&heaterEfficiency, sizeof(heaterEfficiency));
    file.write((const char *)&unitMassInKG, sizeof(unitMassInKG));
    file.write((const char *)&unitSpecificHeat, sizeof(unitSpecificHeat));
    file.write((const char *)&lastINSPosition, sizeof(lastINSPosition));

    file.write((const char *)&lastDMEs, sizeof(lastDMEs));

    file.write((const char *)&expectedBankAngle, sizeof(expectedBankAngle));
    file.write((const char *)&errorRadial, sizeof(errorRadial));
    file.write((const char *)&errorDistance, sizeof(errorDistance));

    file.close();
  }
}