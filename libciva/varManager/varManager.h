#ifndef VAR_MANAGER_H
#define VAR_MANAGER_H

#include <string>
#include <cstdint>
#include <map>

#define VAR_START "LIBCIVA_"

#define DATA_SELECTOR_POS_VAR "DATA_SELECTOR_POS_"
#define MODE_SELECTOR_POS_VAR "MODE_SELECTOR_POS_"
#define STATE_VAR "STATE_"
#define ALIGN_SUBMODE_VAR "ALIGN_SUBMODE_"
#define ACCURACY_INDEX_VAR "ACCURACY_INDEX_"
#define TEMPERATURE_VAR "TEMPERATURE_"
#define OPERATING_TIME_VAR "OPERATING_TIME_"

#define SIM_VAR_AMBIENT_TEMPERATURE "AMBIENT TEMPERATURE"

class VarManager {
    std::string prefix;

protected:
    std::map<std::string, double> store;

public:
    VarManager(const std::string& prefix) : prefix(prefix) {}
    virtual ~VarManager() {}

    virtual void setVar(const std::string& name, double value) noexcept;
    virtual bool getVar(const std::string& name, double& value) noexcept;
};

#endif

