#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>

class Config {
    const std::string& workDir;
    double operatingTempInC = -1;
    double heaterWattage = -1;
    double heaterEfficiency = -1;
    double unitMass = -1;
    double unitSpecificHeat = -1;
    double lastLat = -1;
    double lastLon = -1;

public:
    Config(const std::string& workDir);

    void save() const noexcept;
    double getOperatingTempInC() const noexcept { return operatingTempInC; }
    double getHeaterWattage() const noexcept { return heaterWattage; }
    double getHeaterEfficiency() const noexcept { return heaterEfficiency; }
    double getUnitMass() const noexcept { return unitMass; }
    double getUnitSpecificHeat() const noexcept { return unitSpecificHeat; }
    double getLastLat() const noexcept { return lastLat; }
    double getLastLon() const noexcept { return lastLon; }
};

#endif
