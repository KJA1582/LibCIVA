#include "config.h"

Config::Config(const std::string& workDir): workDir(workDir) {
    std::ifstream file(workDir + "config", std::ios::binary);
    if (file) {
        file.read((char*)&operatingTempInC, sizeof(operatingTempInC));
        file.read((char*)&heaterWattage, sizeof(heaterWattage));
        file.read((char*)&heaterEfficiency, sizeof(heaterEfficiency));
        file.read((char*)&unitMass, sizeof(unitMass));
        file.read((char*)&unitSpecificHeat, sizeof(unitSpecificHeat));
        file.read((char*)&lastLat, sizeof(lastLat));
        file.read((char*)&lastLon, sizeof(lastLon));
        file.close();
    }
}

void Config::save() const noexcept {
    std::ofstream file(workDir + "config", std::ios::binary);
    if (file) {
        file.write((char*)&operatingTempInC, sizeof(operatingTempInC));
        file.write((char*)&heaterWattage, sizeof(heaterWattage));
        file.write((char*)&heaterEfficiency, sizeof(heaterEfficiency));
        file.write((char*)&unitMass, sizeof(unitMass));
        file.write((char*)&unitSpecificHeat, sizeof(unitSpecificHeat));
        file.write((char*)&lastLat, sizeof(lastLat));
        file.write((char*)&lastLon, sizeof(lastLon));
        file.close();
    }
}