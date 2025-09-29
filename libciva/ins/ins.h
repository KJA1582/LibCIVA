#ifndef INS_H
#define INS_H

#include "config/config.h"
#include "varManager/varManager.h"

#define MIN_MODE_8    51
#define MAX_MODE_7    90 // Not specified in manual, but "shortly"
#define MAX_MODE_6   420 // Rest of the ~8.5min MODE 7 and 6
#define MODE_5_TO_0  204 // 3.4min per mode
#define TIME_PER_AI 1200 // 20min per AI, 3 AI per hour, 3h results in AI9

enum class DATA_SELECTOR_POS {
    INV = -1,
    TKGS,
    HDGDA,
    XTKTKE,
    POS,
    WPT,
    DISTIME,
    WIND,
    DSRTKSTS
};

enum class MODE_SELECTOR_POS {
    INV = -1,
    OFF,
    STBY,
    ALIGN,
    NAV,
    ATT,
};

enum class INS_STATE {
    INV = -1,
    OFF,
    STBY,
    ALIGN,
    NAV,
    ATT,
    FAIL,
};

enum class ALIGN_SUBMODE {
    INV = -1,
    MODE_0, // See MODE_4, minimum
    MODE_1, // See MODE_4
    MODE_2, // See MODE_4
    MODE_3, // See MODE_4
    MODE_4, // 3.4min from 4 to 3 etc.
    MODE_5, // Directly at end of MODE_6
    MODE_6, // See MODE_7
    MODE_7, // together with MODE_6, ~8.5min if pres pos was eneterd before MODE_7 finished
    MODE_8, // min 51s, BAT test
    MODE_9, // STBY, change to 8 if ALIGN entered and warmed up
};

enum class ACCURACY_INDEX {
    INV = -1,
    AI_0, // MIN
    AI_1,
    AI_2, // MIN if DME alt invalid
    AI_3,
    AI_4,
    AI_5,
    AI_6,
    AI_7,
    AI_8,
    AI_9, // MAX, starts here
};

enum class PERFORMANCE_INDEX {
    INV = -1,
    PI_1, // Eradication
    PI_4, // Aided
    PI_5, // Unaided
};

class INS {
    VarManager& varManager;
    const std::string id;
    const Config config;

    void setINSState(INS_STATE state) const noexcept;
    INS_STATE getINSState() const noexcept;

    void setTemperature(double temperature) const noexcept;
    double getTemperature() const noexcept;

    void setOperatingTime(double operatingTime) const noexcept;
    double getOperatingTime() const noexcept;

public:
    INS(VarManager& varManager, const Config& config, const std::string& id);
    ~INS() {};

    void update(double dTime) noexcept;

    void setDataSelectorPos(DATA_SELECTOR_POS pos) const noexcept;
    DATA_SELECTOR_POS getDataSelectorPos() const noexcept;
    void incDataSelectorPos() const noexcept;
    void decDataSelectorPos() const noexcept;

    void setModeSelectorPos(MODE_SELECTOR_POS pos) const noexcept;
    MODE_SELECTOR_POS getModeSelectorPos() const noexcept;
    void incModeSelectorPos() const noexcept;
    void decModeSelectorPos() const noexcept;

    void setAlignSubmode(ALIGN_SUBMODE mode) const noexcept;
    ALIGN_SUBMODE getAlignSubmode() const noexcept;

    void setAccuracyIndex(ACCURACY_INDEX index) const noexcept;
    ACCURACY_INDEX getAccuracyIndex() const noexcept;
};

#endif