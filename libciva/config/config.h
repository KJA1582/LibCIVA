#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>

#include "geoutils/geoutils.h"

class Config {
  const std::string workDir;
  double operatingTempInC = 76;
  double heaterWattage = 2000;
  double heaterEfficiency = 0.7;
  double unitMass = 5;
  double unitSpecificHeat = 900;
  POSITION lastINSPosition = { 999, 999 };

public:
  Config(const std::string &workDir) noexcept;

  void save() const noexcept;
  
  inline double getOperatingTempInC() const noexcept {
    return operatingTempInC;
  }
  inline double getHeaterWattage() const noexcept {
    return heaterWattage;
  }
  inline double getHeaterEfficiency() const noexcept {
    return heaterEfficiency;
  }
  inline double getUnitMass() const noexcept {
    return unitMass;
  }
  inline double getUnitSpecificHeat() const noexcept {
    return unitSpecificHeat;
  }
  inline POSITION getLastINSPosisiton() const noexcept {
    return lastINSPosition;
  }

  inline void setLastLat(double lat) noexcept {
    lastINSPosition.latitude = lat;
  }
  inline void setLastLon(double lon) noexcept {
    lastINSPosition.longitude = lon;
  }
  inline void setLastINSPosition(POSITION pos) noexcept {
    lastINSPosition = pos;
  }
};

#endif
