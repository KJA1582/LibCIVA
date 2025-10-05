#ifndef INS_H
#define INS_H

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "config/config.h"
#include "geoutils/geoutils.h"
#include "varManager/varManager.h"

constexpr auto MIN_MODE_8 = 51;
constexpr auto MAX_MODE_7 = 90; // Not specified in manual, but "shortly"
constexpr auto MAX_MODE_6 = 420; // Rest of the ~8.5min MODE 7 and 6
constexpr auto MODE_5_TO_0 = 204; // 3.4min per mode
constexpr auto TIME_PER_AI = 1200; // 20min per AI, 3 AI per hour, 3h results in AI9
constexpr auto MAX_BAT_TEST_TIME = 12; // 12s bat test

constexpr char PROG_NUM[] = { 1, 1, 0, 7 }; // CIV-A-22

constexpr auto MIN_GS = 75;
constexpr auto MIN_TAS_WIND = 115;
constexpr auto MAX_TAS_WIND = 606;
constexpr auto MAX_RAMP_DEV = 76;
constexpr auto MAX_DEV = 33;
constexpr auto MAX_GS = 910;
constexpr auto MAX_GS_DISPLAY = 2400;
constexpr auto MAX_DRIFT_ANGLE = 45;
constexpr auto INTIAL_TIME_IN_NAV = 5400;

constexpr auto DISPLAY_CHAR_RIGHT = 10;
constexpr auto DISPLAY_CHAR_LEFT = 11;
constexpr auto DISPLAY_CHAR_BLANK = 12;

enum class DATA_SELECTOR: uint8_t {
  TKGS,
  HDGDA,
  XTKTKE,
  POS,
  WPT,
  DISTIME,
  WIND,
  DSRTKSTS
};

enum class MODE_SELECTOR: uint8_t {
  OFF,
  STBY,
  ALIGN,
  NAV,
  ATT,
};

enum class INS_STATE: uint8_t {
  OFF,
  STBY,
  ALIGN,
  NAV,
  ATT,
  FAIL,
};

enum class ALIGN_SUBMODE: uint8_t {
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

enum class ACTION_MALFUNCTION_CODE: int8_t {
  INV = -1,
  A02_31, // ground speed > 910, non-clearable
  A02_42, // drift angle > 45, non-clearable
  A02_49, // pos update in flight > 33, non-clearable
  A02_63, // self checks failed, non-clearable
  A04_41, // ramp pos > 76nmi from last pos, clearable
  A04_43, // ramp pos missmatch between pairs of units, non-clearable
  A04_57, // taxi during align, non-clearable
};

enum class BATTERY_TEST: uint8_t {
  IDLE,
  RUNNING,
  COMPLETED,
  FAILED,
  INHIBITED,
};

enum class INSERT_MODE: int8_t {
  INV = -1,
  POS_LAT,
  PRE_POS_LON,
  POS_LON,
  WPT_LAT,
  WPT_LON,
  DME_FREQ,
  DME_ALT,
  PERFORMANCE_INDEX,
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
// Characters are encoded 0->9 for 0->9, 10 for R, 11 for L, 12 for " "
// TO/FROM 11 indicates blinking (to display: if > 11 then value-1 and blink else value and no blink)
typedef union {
  double value;
  struct {
    uint8_t LEFT_1 : 4;
    uint8_t LEFT_2 : 4;
    uint8_t LEFT_3 : 4;
    uint8_t LEFT_4 : 4;
    uint8_t LEFT_5 : 4;
    bool LEFT_DEG_1 : 1;
    bool LEFT_DEG_2 : 1;
    bool LEFT_DEC_1 : 1;
    bool LEFT_DEC_2 : 1;
    uint8_t RIGHT_1 : 4;
    uint8_t RIGHT_2 : 4;
    uint8_t RIGHT_3 : 4;
    uint8_t RIGHT_4 : 4;
    uint8_t RIGHT_5 : 4;
    uint8_t RIGHT_6 : 4;
    uint8_t TO : 4;
    uint8_t FROM : 4;
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

typedef struct {
  POSITION position;
  // In MHz multiplied by 100
  uint16_t frequency;
  // In thousands of feet
  uint8_t altitude;
} DME;

class INS {
  // Global vars manager
  VarManager &varManager;
  // INS Config
  Config config;
  // Unit ID
  const std::string id;
  // List of active malfunctions
  std::vector<ACTION_MALFUNCTION_CODE> actionMalfunctionCodes;

  #pragma region Positions

  // Current position displayed on unit
  POSITION displayPosition = { 999, 999 };
  // INS Position without any updates
  POSITION initialINSPosition = { 999, 999 };
  // Current INS position with updates
  POSITION currentINSPosition = { 999, 999 };

  #pragma endregion

  #pragma region Waypoints/DMEs

  // Waypoints (0 is not settable by hand)
  POSITION waypoints[10] = {
    { 999, 999 }, { 999, 999 }, { 999, 999 }, { 999, 999 }, { 999, 999 }, { 999, 999 },
    { 999, 999 } , { 999, 999 } , { 999, 999 } , { 999, 999 }
  };
  // DMEs, index of selector - 1 = array index
  DME dmes[9] = {
    { 999, 999 }, { 999, 999 }, { 999, 999 }, { 999, 999 }, { 999, 999 }, { 999, 999 },
    { 999, 999 } , { 999, 999 } , { 999, 999 }
  };

  #pragma endregion

  #pragma region Indicators/Display

  // Active indicators
  INDICATORS indicators = { 0 };
  // Previous indicators for test mode
  INDICATORS previousIndicators = { 0 };
  // Current Display
  DISPLAY display = { 0 };

  #pragma endregion

  #pragma region States

  // Current oven temperature
  double ovenTemperature = 0;
  // Current time in mode
  double timeInMode = 0;
  // Current time in NAV, starts at AI 5 value, ticks down in algin after AI 5, ticks up in NAV
  double timeInNAV = INTIAL_TIME_IN_NAV;
  // Current track
  double track = 0;
  // Current INS State
  INS_STATE state = INS_STATE::OFF;
  // Current align submode
  ALIGN_SUBMODE alignSubmode = ALIGN_SUBMODE::MODE_9;
  // State of battery test
  BATTERY_TEST batteryTest = BATTERY_TEST::IDLE;
  // Position of data selector
  DATA_SELECTOR dataSelector = { DATA_SELECTOR::POS };
  // Position of mode selector
  MODE_SELECTOR modeSelector = { MODE_SELECTOR::OFF };
  // Current active insert mode
  INSERT_MODE insertMode = { INSERT_MODE::INV };
  // Current performance index (4, 5)
  uint8_t activePerformanceIndex = 5;
  // PI displayed (required due to PI 1 never being active but displayed)
  // 1 -> Eradication, 4 -> aided (DME/triple mix), 5 -> start, unaided
  uint8_t displayPerformanceIndex = 5;
  // Current accuracy index, 0 to 9, 0 best, 9 worst, 2 if DME update altitude invalid
  uint8_t accuracyIndex = 9;
  // CHARS read for insert mode
  uint8_t charactersRead = 0;
  // Waypoint selector
  uint8_t waypointSelector = 0;
  // Displayed action malfunction code index
  uint8_t displayActionMalfunctionCodeIndex = 0;
  // Leg
  uint8_t currentLegStart = 1;
  uint8_t currentLegEnd = 2;
  // Malfunction code displayed if true, action code otherwise
  bool mafunctionCodeDisplayed = false;
  // Test mode
  bool inTestMode = false;

  #pragma endregion

  #pragma region Private Getter/Setter

  void addActionMalfunctionCode(const ACTION_MALFUNCTION_CODE code) noexcept;
  bool removeCurrentActionMalfunctionCode() noexcept;
  void clearActionMalfunctionCodes() noexcept;
  void incCurrentActionMalfunctionCode() noexcept;
  bool hasActionMalfunctionCode(const ACTION_MALFUNCTION_CODE code) const noexcept;
  bool hasActionMalfunctionCode() const noexcept;
  ACTION_MALFUNCTION_CODE getCurrentActionMalfunctionCode() const noexcept;

  #pragma endregion

  void temperatureSim(const double dTime) noexcept;
  void reset(const bool full) noexcept;
  void calculateTrack() noexcept;
  void handleOutOfBounds() noexcept;
  void updateDisplay() noexcept;
  void align(const double dTime) noexcept;
  void exportVars() const noexcept;

  inline void clearDisplay() noexcept {
    uint64_t d;
    d = 0x00CCCCCCCC0CCCCC;
    display = *(reinterpret_cast<DISPLAY *>(&d));
  }

public:
  INS(VarManager &varManager, const std::string &id, const std::string &workDir) noexcept;
  ~INS() noexcept;

  void update(const double dTime) noexcept;

  #pragma region Public Getter/Setter

  inline void setDataSelectorPos(const DATA_SELECTOR pos) noexcept {
    dataSelector = pos;
  }
  inline void setModeSelectorPos(const MODE_SELECTOR pos) noexcept {
    modeSelector = pos;
  }

  inline DATA_SELECTOR getDataSelectorPos() const noexcept {
    return dataSelector;
  }
  inline MODE_SELECTOR getModeSelectorPos() const noexcept {
    return modeSelector;
  }

  #pragma endregion

  #pragma region Events

  void incDataSelectorPos() noexcept;
  void incModeSelectorPos() noexcept;
  void incWaypointSelectorPos() noexcept;

  void decDataSelectorPos() noexcept;
  void decModeSelectorPos() noexcept;
  void decWaypointSelectorPos() noexcept;

  void handleNumeric(const uint8_t value) noexcept;
  void handleInsert() noexcept;
  void handleTestButtonState(const bool state) noexcept;

  #pragma endregion
};

#endif