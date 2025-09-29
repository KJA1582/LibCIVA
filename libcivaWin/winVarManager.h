#ifndef WIN_VAR_MANAGER_H
#define WIN_VAR_MANAGER_H

#include <iomanip>
#include <iostream>


#include <libciva.h>

class WinVarManager : public VarManager {
public:
    WinVarManager() : VarManager("") {}

    void setVar(const std::string& name, double value) noexcept final;
    bool getVar(const std::string& name, double& value) noexcept final;

    void dump() const noexcept;
};

#endif