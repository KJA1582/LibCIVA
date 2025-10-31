#ifndef INS_H
#define INS_H

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "config/config.h"
#include "types/types.h"
#include "varManager/varManager.h"

constexpr auto MIN_MODE_8 = 51;
constexpr auto MAX_MODE_7 = 90; // Not specified in manual, but "shortly"
constexpr auto MAX_MODE_6 = 420; // Rest of the ~8.5min MODE 7 and 6
constexpr auto MODE_5_TO_0 = 204; // 3.4min per mode
constexpr auto TIME_PER_AI = 1200; // 20min per AI, 3 AI per hour, 3h results in AI9
constexpr auto MAX_BAT_TEST_TIME = 12; // 12s bat test

constexpr uint8_t PROG_NUM[] = { 1, 1, 0, 7 }; // CIV-A-22

constexpr auto MIN_GS = 75;
constexpr auto MIN_GS_TIME = 10;
constexpr auto MIN_TAS_WIND = 115;
constexpr auto MAX_TAS_WIND = 606;
constexpr auto MAX_RAMP_DEV = 76;
constexpr auto MAX_DEV = 33;
constexpr auto MAX_GS = 910;
constexpr auto DRIFT_GS = 500;
constexpr auto MAX_GS_DISPLAY = 2400;
constexpr auto MAX_DRIFT_ANGLE = 45;
constexpr auto INTIAL_TIME_IN_NAV = 5400;

constexpr auto DISPLAY_CHAR_RIGHT = 10;
constexpr auto DISPLAY_CHAR_LEFT = 11;
constexpr auto DISPLAY_CHAR_BLANK = 12;

class INS {
  // Global vars manager
  VarManager &varManager;
  // INS Config
  Config config;
  // Unit ID
  const std::string id;
  // List of active malfunctions
  ACTION_MALFUNCTION_CODES actionMalfunctionCodes;

  #pragma region Positions

  // Current position displayed on unit
  POSITION displayPosition = { 999, 999 };
  // INS Position without any updates
  POSITION initialINSPosition = { 999, 999 };
  // Current INS position with updates
  POSITION currentINSPosition = { 999, 999 };
  // HOLD mode
  POSITION holdINSPosition = { 999, 999 };
  POSITION holdPosition = { 999, 999 };
  // Delta to actual sim position
  POSITION simPosDelta = { 0, 0 };
  // Error
  POSITION initialError = { 0, 0 };
  POSITION currentError = { 0, 0 };

  #pragma endregion

  #pragma region Waypoints/DMEs

  // Waypoints (0 is not settable by hand)
  POSITION waypoints[10] = {
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } , { 0, 0 } , { 0, 0 } , { 0, 0 }
  };
  // DMEs, index of selector - 1 = array index
  DME DMEs[9] = {
    { 0, 0, 0, 0}, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },  { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
    { 0, 0, 0, 0 } , { 0, 0, 0, 0 } , { 0, 0, 0, 0 }
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
  // Uncorrected time in NAV, starts at AI 5 value, ticks down in algin after AI 5, ticks up in NAV
  double initialTimeInNAV = INTIAL_TIME_IN_NAV;
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
  // Current accuracy index, 0 to 9, 0 best, 9 worst, 2 if DME update altitude invalid
  uint8_t accuracyIndex = 9;
  // CHARS read for insert mode
  uint8_t charactersRead = 0;
  // Waypoint selector
  uint8_t waypointSelector = 0;
  // Displayed action malfunction code index
  // MAKE SURE THIS MATCHES actionmalfunctionCodes.code AT ALL TIMES
  uint8_t displayActionMalfunctionCodeIndex = 0;
  // Leg
  uint8_t currentLegStart = 1;
  uint8_t currentLegEnd = 2;
  // Active DME index, 0 indicates no active DME
  uint8_t activeDME = 0;
  // DME mode
  DME_MODE dmeMode = DME_MODE::INV;
  // Malfunction code displayed if true, action code otherwise
  bool mafunctionCodeDisplayed = false;
  // Test mode
  bool inTestMode = false;
  // HOLD
  bool inHoldMode = false;
  bool holdModeInserted = false;

  #pragma endregion

  #pragma region Private Getter/Setter

  void advanceActionMalfunctionIndex() noexcept;
  void updateSimPosDelta() noexcept;
  void updateCurrentINSPosition(const double dTime) noexcept;

  #pragma endregion

  void temperatureSim(const double dTime) noexcept;
  void reset(const bool full) noexcept;
  void calculateTrack() noexcept;
  void handleOutOfBounds() noexcept;
  void updateDisplay() noexcept;
  void align(const double dTime) noexcept;
  void exportVars() const noexcept;

  void formatActionMalfunctionCode(const bool showingMalf) noexcept;

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
  void handleDMEModeEntry(const uint8_t value) noexcept;
  void handleClear() noexcept;
  void handleWaypointChange() noexcept;
  void handleHoldButton() noexcept;

  #pragma endregion
};

#endif