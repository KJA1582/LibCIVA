#include "ins.h"

#pragma region Helpers

double heater(const Config& config, double currentTempC, double ambientTempC, bool shouldHeat,
              double dTime)
{
    if (shouldHeat && currentTempC >= config.getOperatingTempInC()) return currentTempC;
    if (!shouldHeat && currentTempC <= ambientTempC + 0.1)
        return ambientTempC;

    double cooling = 0.02;
    double loss = cooling * (currentTempC - ambientTempC) * (dTime / config.getUnitMass());

    if (!shouldHeat) return currentTempC - loss;

double energy = config.getHeaterWattage() * config.getHeaterEfficiency() * dTime;
double dTemp = energy / (config.getUnitMass() * config.getUnitSpecificHeat());

return currentTempC + dTemp - loss;
}

#pragma endregion

INS::INS(VarManager& varManager, const Config& config, const std::string& id) :
    varManager(varManager), id(id), config(config) {
    setINSState(INS_STATE::OFF);
    setModeSelectorPos(MODE_SELECTOR_POS::OFF);
    setDataSelectorPos(DATA_SELECTOR_POS::POS);
    setAlignSubmode(ALIGN_SUBMODE::MODE_9);
    setAccuracyIndex(ACCURACY_INDEX::AI_9);

    double temperature;
    varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, temperature);
    setTemperature(temperature);

    setOperatingTime(0);
}

void INS::update(double dTime) noexcept {
    double operatingTime = getOperatingTime();

    // Switching to ATT
    if (getModeSelectorPos() == MODE_SELECTOR_POS::ATT) {
        setINSState(INS_STATE::ATT);

        operatingTime = 0;
        return;
    }

    INS_STATE state = getINSState();
    MODE_SELECTOR_POS mode = getModeSelectorPos();

    switch (state) {
        case INS_STATE::INV: {
            // ERROR CASE
            break;
        }
        case INS_STATE::OFF: {
            if (mode != MODE_SELECTOR_POS::OFF) {
                // Upmode
                setINSState(INS_STATE::STBY);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }
            break;
        }
        case INS_STATE::STBY: {
            if (mode == MODE_SELECTOR_POS::OFF) {
                // Downmode
                setINSState(INS_STATE::OFF);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }
            else if (mode != MODE_SELECTOR_POS::STBY) {
                // Upmode
                setINSState(INS_STATE::ALIGN);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }

            operatingTime += dTime;
            break;
        }
        case INS_STATE::ALIGN: {
            ALIGN_SUBMODE submode = getAlignSubmode();

            if (mode == MODE_SELECTOR_POS::OFF) {
                // Downmode
                setINSState(INS_STATE::OFF);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }
            else if (mode == MODE_SELECTOR_POS::STBY) {
                // Downmode
                setINSState(INS_STATE::STBY);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }
            else if (mode == MODE_SELECTOR_POS::NAV) {
                // Upmode
                if (submode <= ALIGN_SUBMODE::MODE_5) {
                    setINSState(INS_STATE::NAV);
                    // FIXME: To truly get the downside of AI5 NAV, save the OG AI on nav entry as a scalar
                    // Meaning, at AI0, error increases less fast than at AI5. POS updating will reset scalar and AI to AI0 level
                    setAccuracyIndex(ACCURACY_INDEX::AI_0);

                    operatingTime = 0;
                    break;
                }
            }

            switch (submode) {
                case ALIGN_SUBMODE::INV: {
                    // ERROR CASE
                    break;
                }
                case ALIGN_SUBMODE::MODE_9: {
                    if (getTemperature() >= config.getOperatingTempInC()) {
                        setAlignSubmode(ALIGN_SUBMODE::MODE_8);
                        operatingTime = 0;
                    }
                    break;
                }
                case ALIGN_SUBMODE::MODE_8: {
                    // TODO: Battery test is in here, 12s, immediate on entry, only if selector is not in NAV

                    if (operatingTime >= MIN_MODE_8) {
                        setAlignSubmode(ALIGN_SUBMODE::MODE_7);
                        operatingTime = 0;
                    }
                    break;
                }
                case ALIGN_SUBMODE::MODE_7: {
                    // FIXME: PRES POS must be entered before MODE_6 is activated
                    if (operatingTime >= MAX_MODE_7) {
                        setAlignSubmode(ALIGN_SUBMODE::MODE_6);
                        operatingTime = 0;
                    }
                    break;
                }
                case ALIGN_SUBMODE::MODE_6: {
                    if (operatingTime >= MAX_MODE_6) {
                        setAlignSubmode(ALIGN_SUBMODE::MODE_5);
                        operatingTime = 0;
                    }
                    break;
                }
                case ALIGN_SUBMODE::MODE_5:
                case ALIGN_SUBMODE::MODE_4:
                case ALIGN_SUBMODE::MODE_3:
                case ALIGN_SUBMODE::MODE_2:
                case ALIGN_SUBMODE::MODE_1: {
                    if (operatingTime >= MODE_5_TO_0) {
                        setAlignSubmode((ALIGN_SUBMODE)((int)submode - 1));
                        operatingTime = 0;
                    }
                    break;
                }
                case ALIGN_SUBMODE::MODE_0: {
                    // just sit here
                    break;
                }
            }

            operatingTime += dTime;
            break;
        }
        case INS_STATE::NAV: {
            if (mode == MODE_SELECTOR_POS::OFF) {
                // Downmode
                setINSState(INS_STATE::OFF);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }
            else if (mode == MODE_SELECTOR_POS::STBY) {
                // Downmode
                setINSState(INS_STATE::STBY);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            } 
            else if (mode == MODE_SELECTOR_POS::ALIGN) {
                // Downmode
                setINSState(INS_STATE::ALIGN);
                // FIXME: Unsure if this downmodes to submode 9
                // It does not for 0, no idea what happens if at > 5
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }
            
            // TODO: NAV flow
            ACCURACY_INDEX accuracyIndex = getAccuracyIndex();
            if (operatingTime >= TIME_PER_AI && accuracyIndex < ACCURACY_INDEX::AI_9) {
                setAccuracyIndex((ACCURACY_INDEX)((int)accuracyIndex + 1));
                operatingTime = 0;
            }

            operatingTime += dTime;
            break;
        }
        case INS_STATE::ATT: {
            if (getModeSelectorPos() == MODE_SELECTOR_POS::OFF) {
                // Downmode
                setINSState(INS_STATE::OFF);
                setAlignSubmode(ALIGN_SUBMODE::MODE_9);
            }

            operatingTime += dTime;
            return;
        }
    }

    double ambient = 0;
    if (varManager.getVar(SIM_VAR_AMBIENT_TEMPERATURE, ambient)) {
        setTemperature(heater(config, getTemperature(), ambient, state > INS_STATE::OFF, dTime));
    }

    setOperatingTime(operatingTime);
}

#pragma region State setters and getters

void INS::setINSState(INS_STATE state) const noexcept {
    varManager.setVar(STATE_VAR + id, (double)state);
}
INS_STATE INS::getINSState() const noexcept {
    double state;
    if (varManager.getVar(STATE_VAR + id, state)) return (INS_STATE)state;

    return INS_STATE::INV;
}

void INS::setTemperature(double temperature) const noexcept {
    varManager.setVar(TEMPERATURE_VAR + id, temperature);
}
double INS::getTemperature() const noexcept {
    double temperature;
    varManager.getVar(TEMPERATURE_VAR + id, temperature);
    return temperature;
}

void INS::setOperatingTime(double operatingTime) const noexcept {
    varManager.setVar(OPERATING_TIME_VAR + id, operatingTime);
}
double INS::getOperatingTime() const noexcept {
    double operatingTime;
    varManager.getVar(OPERATING_TIME_VAR + id, operatingTime);
    return operatingTime;
}

void INS::setDataSelectorPos(DATA_SELECTOR_POS pos) const noexcept {
    varManager.setVar(DATA_SELECTOR_POS_VAR + id, (double)pos);
}
DATA_SELECTOR_POS INS::getDataSelectorPos() const noexcept {
    double pos;
    if (varManager.getVar(DATA_SELECTOR_POS_VAR + id, pos)) return (DATA_SELECTOR_POS)pos;

    return DATA_SELECTOR_POS::INV;  
}
void INS::incDataSelectorPos() const noexcept {
    DATA_SELECTOR_POS state = getDataSelectorPos();
    if (state != DATA_SELECTOR_POS::INV && state != DATA_SELECTOR_POS::DSRTKSTS) {
        setDataSelectorPos((DATA_SELECTOR_POS)((int)state + 1));
    }
}
void INS::decDataSelectorPos() const noexcept {
    DATA_SELECTOR_POS state = getDataSelectorPos();
    if (state != DATA_SELECTOR_POS::INV && state != DATA_SELECTOR_POS::TKGS) {
        setDataSelectorPos((DATA_SELECTOR_POS)((int)state - 1));
    }
}

void INS::setModeSelectorPos(MODE_SELECTOR_POS pos) const noexcept {
    varManager.setVar(MODE_SELECTOR_POS_VAR + id, (double)pos);
}
MODE_SELECTOR_POS INS::getModeSelectorPos() const noexcept {
    double pos;
    if (varManager.getVar(MODE_SELECTOR_POS_VAR + id, pos)) return (MODE_SELECTOR_POS)pos;

    return MODE_SELECTOR_POS::INV;
}
void INS::incModeSelectorPos() const noexcept {
    MODE_SELECTOR_POS state = getModeSelectorPos();
    if (state != MODE_SELECTOR_POS::INV && state != MODE_SELECTOR_POS::ATT) {
        setModeSelectorPos((MODE_SELECTOR_POS)((int)state + 1));
    }
}
void INS::decModeSelectorPos() const noexcept {
    MODE_SELECTOR_POS state = getModeSelectorPos();
    if (state != MODE_SELECTOR_POS::INV && state != MODE_SELECTOR_POS::OFF) {
        setModeSelectorPos((MODE_SELECTOR_POS)((int)state - 1));
    }
}

void INS::setAlignSubmode(ALIGN_SUBMODE mode) const noexcept {
    varManager.setVar(ALIGN_SUBMODE_VAR + id, (double)mode);
}
ALIGN_SUBMODE INS::getAlignSubmode() const noexcept {
    double mode;
    if (varManager.getVar(ALIGN_SUBMODE_VAR + id, mode)) return (ALIGN_SUBMODE)mode;

    return ALIGN_SUBMODE::INV;
}

void INS::setAccuracyIndex(ACCURACY_INDEX index) const noexcept {
    varManager.setVar( ACCURACY_INDEX_VAR + id, (double)index);
}
ACCURACY_INDEX INS::getAccuracyIndex() const noexcept {
    double index;
    if (varManager.getVar(ACCURACY_INDEX_VAR + id, index)) return (ACCURACY_INDEX)index;

    return ACCURACY_INDEX::INV;
}

#pragma endregion
