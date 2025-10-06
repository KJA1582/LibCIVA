#ifndef CONFIG_H
#define CONFIG_H

#include <cstring>
#include <fstream>
#include <string>

#include "types/types.h"

class Config {
  const std::string basePath;
  double operatingTempInC = 76;
  double heaterWattage = 2000;
  double heaterEfficiency = 0.7;
  double unitMass = 5;
  double unitSpecificHeat = 900;
  POSITION lastINSPosition = { 999, 999 };
  DME lastDMEs[9] = {
    { 0, 0, 0, 0}, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
    { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }
  };

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
  inline void getLastDMEs(DME(&DMEs)[9]) const noexcept {
    std::memcpy(DMEs, lastDMEs, sizeof(lastDMEs));
  }

  inline void setLastINSPosition(POSITION pos) noexcept {
    lastINSPosition = pos;
  }
  inline void setLastDMEs(const DME(&DMEs)[9]) noexcept {
    std::memcpy(lastDMEs, DMEs, sizeof(DMEs));
  }
};

#endif
