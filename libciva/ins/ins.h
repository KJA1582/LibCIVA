#ifndef INS_H
#define INS_H

#include "config/config.h"
#include "geoutils/geoutils.h"
#include "varManager/varManager.h"

constexpr auto MIN_MODE_8 = 51;
constexpr auto MAX_MODE_7 = 90; // Not specified in manual, but "shortly"
constexpr auto MAX_MODE_6 = 420; // Rest of the ~8.5min MODE 7 and 6
constexpr auto MODE_5_TO_0 = 204; // 3.4min per mode
constexpr auto TIME_PER_AI = 1200; // 20min per AI, 3 AI per hour, 3h results in AI9

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

enum class ACTION_MALFUNCTION_CODE {
  INV = -1,
  A04_45, // ramp pos out of bounds
  A04_57, // taxi during align
  A06_41, // ramp pos > 76nmi from last pos
  A06_43, // ramp pos missmatch between pairs of units
  A06_56, // inertial pos - dispalyed pos > 3+3*t
};

enum class BATTERY_TEST {
  IDLE,
  RUNNING,
  COMPLETED,
  FAILED,
  INHIBITED,
};

enum class INSERT_MODE {
  INV = -1,
  POS_LAT,
  PRE_POS_LON,
  POS_LON,
  WPT_LAT,
  WPT_LON,
  DME_FREQ,
  DME_ALT,
};

// Use Bitset to set/reset indicators
typedef union {
  double value;
  struct {
    bool MSU_BAT : 1;
    bool READY_NAV : 1;
    bool HOLD : 1;
    bool REMOTE : 1;
    bool INSERT : 1;
    bool ALERT : 1;
    bool CDU_BAT : 1;
    bool WARN : 1;
    bool WAYPOINT_CHANGE : 1;
  } indicator;
} INDICATORS;

// Use bitset to set display characters
// Characters are encoded 0->9 for 0->9, 10 for R, 11 for L
// TO/FROM 11 indicates blinking (value is -1)
typedef union {
  double value;
  struct {
    unsigned char LEFT_1 : 4;
    unsigned char LEFT_2 : 4;
    unsigned char LEFT_3 : 4;
    unsigned char LEFT_4 : 4;
    unsigned char LEFT_5 : 4;
    bool LEFT_DEG_1 : 1;
    bool LEFT_DEG_2 : 1;
    bool LEFT_DEC_1 : 1;
    bool LEFT_DEC_2 : 1;
    unsigned char RIGHT_1 : 4;
    unsigned char RIGHT_2 : 4;
    unsigned char RIGHT_3 : 4;
    unsigned char RIGHT_4 : 4;
    unsigned char RIGHT_5 : 4;
    unsigned char RIGHT_6 : 4;
    unsigned char TO : 4;
    unsigned char FROM : 4;
    bool RIGHT_DEG_1 : 1;
    bool RIGHT_DEG_2 : 1;
    bool RIGHT_DEC_1 : 1;
    bool RIGHT_DEC_2 : 1;
    bool N : 1;
    bool S : 1;
    bool E : 1;
    bool W : 1;
  } characters;
} DISPLAY;

class INS {
  VarManager &varManager;
  Config config;

  const std::string id;

  void reset(bool full) const noexcept;

  #pragma region Getter/Setter

  void setINSState(INS_STATE state) const noexcept;
  INS_STATE getINSState() const noexcept;

  void setTemperature(double temperature) const noexcept;
  double getTemperature() const noexcept;

  void setOperatingTime(double operatingTime) const noexcept;
  double getOperatingTime() const noexcept;

  void setAlignSubmode(ALIGN_SUBMODE mode) const noexcept;
  ALIGN_SUBMODE getAlignSubmode() const noexcept;

  void setAccuracyIndex(ACCURACY_INDEX index) const noexcept;
  ACCURACY_INDEX getAccuracyIndex() const noexcept;

  void setDisplayPosLat(double lat) const noexcept;
  double getDisplayPosLat() const noexcept;
  void setDisplayPosLon(double lon) const noexcept;
  double getDisplayPosLon() const noexcept;

  void setActionMalfunctionCode(ACTION_MALFUNCTION_CODE code) const noexcept;

  void setIndicators(INDICATORS indicators) const noexcept;

  void setBatteryTestState(BATTERY_TEST state) const noexcept;
  BATTERY_TEST getBatteryTestState() const noexcept;

  void setDisplay(DISPLAY display) const noexcept;

  void setInsertMode(INSERT_MODE mode) const noexcept;
  INSERT_MODE getInsertMode() const noexcept;

  void setINSPosLat(double lat) const noexcept;
  double getINSPosLat() const noexcept;
  void setINSPosLon(double lon) const noexcept;
  double getINSPosLon() const noexcept;

  #pragma endregion

  double align() noexcept;
  void display() const noexcept;
public:
  INS(VarManager &varManager, const std::string &id, const std::string &workDir) noexcept;
  ~INS() noexcept;

  void update(double dTime) noexcept;

  #pragma region Getter/Setter

  void setDataSelectorPos(DATA_SELECTOR_POS pos) const noexcept;
  DATA_SELECTOR_POS getDataSelectorPos() const noexcept;

  void setModeSelectorPos(MODE_SELECTOR_POS pos) const noexcept;
  MODE_SELECTOR_POS getModeSelectorPos() const noexcept;

  ACTION_MALFUNCTION_CODE getActionMalfunctionCode() const noexcept;

  INDICATORS getIndicators() const noexcept;

  DISPLAY getDisplay() const noexcept;

  #pragma endregion

  #pragma region Events

  void incDataSelectorPos() const noexcept;
  void decDataSelectorPos() const noexcept;

  void incModeSelectorPos() const noexcept;
  void decModeSelectorPos() const noexcept;

  void handleNumeric(char value) const noexcept;
  void handleInsert() const noexcept;

  #pragma endregion
};

#endif