#ifndef CONFIG_H
#define CONFIG_H

#include <cstring>
#include <fstream>
#include <string>

#include "types/types.h"

class Config {
  const std::string basePath;
  const std::string id;
  double operatingTempInC = 76;
  double heaterWattage = 2000;
  double heaterEfficiency = 0.9;
  double unitMassInKG = 5;
  double unitSpecificHeat = 900;
  POSITION lastINSPosition = { 999, 999 };
  DME lastDMEs[9] = {
    { 0, 0, 0, 0}, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
    { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }
  };
  char expectedBankAngle = 30;

public:
  Config(const std::string &workDir, const std::string &id) noexcept;

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
    return unitMassInKG;
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
  inline char getExpectedBankAngle() const noexcept {
    return expectedBankAngle;
  }

  inline void setLastINSPosition(POSITION pos) noexcept {
    lastINSPosition = pos;
  }
  inline void setLastDMEs(const DME(&DMEs)[9]) noexcept {
    std::memcpy(lastDMEs, DMEs, sizeof(DMEs));
  }
};

#endif
