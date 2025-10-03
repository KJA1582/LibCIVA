#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>

class Config {
  const std::string workDir;
  double operatingTempInC = 76;
  double heaterWattage = 2000;
  double heaterEfficiency = 0.7;
  double unitMass = 5;
  double unitSpecificHeat = 900;
  double lastLat = 999;
  double lastLon = 999;

public:
  Config(const std::string &workDir) noexcept;

  void save() const noexcept;
  double getOperatingTempInC() const noexcept { return operatingTempInC; }
  double getHeaterWattage() const noexcept { return heaterWattage; }
  double getHeaterEfficiency() const noexcept { return heaterEfficiency; }
  double getUnitMass() const noexcept { return unitMass; }
  double getUnitSpecificHeat() const noexcept { return unitSpecificHeat; }

  void setLastLat(double lat) noexcept { lastLat = lat; }
  double getLastLat() const noexcept { return lastLat; }
  
  void setLastLon(double lon) noexcept { lastLon = lon; }
  double getLastLon() const noexcept { return lastLon; }
};

#endif
