#include "winVarManager.h"

void WinVarManager::setVar(const std::string& name, double value) noexcept {
    VarManager::setVar(name, value);
}

bool WinVarManager::getVar(const std::string& name, double& value) noexcept {
    return VarManager::getVar(name, value);
}

void WinVarManager::dump() const noexcept {
    for (auto it = store.begin(); it != store.end(); ++it) {
        std::cout << std::left << std::setfill(' ') << std::setw(40) << it->first;
        std::cout << " = " << std::right << std::setfill(' ') << std::setw(10) << it->second;
        std::cout << std::endl;
    }
}