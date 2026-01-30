#ifndef INS_H
#define INS_H

#ifndef __INTELLISENSE__
#	define MODULE_EXPORT __attribute__( ( visibility( "default" ) ) )
#	define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#	define MODULE_EXPORT
#	define MODULE_WASM_MODNAME(mod)
#	define __attribute__(x)
#	define __restrict__
#endif

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "config/config.h"
#include "logger/logger.h"
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
constexpr auto INITIAL_TIME_IN_NAV = 5400;
constexpr auto MIN_LEG_TIME = 25.6;
constexpr auto LEG_TIME_ALERT = 120;
constexpr auto MIN_DME_TIME = 12; // After this, DME indicator can go green
constexpr auto MAX_SINGLE_DME_TIME_UNTIL_VALID = 150; // Stop DME update if not valid below this
constexpr auto MAX_DUAL_DME_TIME_UNTIL_VALID = 300; // Stop DME update if nopt valid below this

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
  // Current INS tripple mix position with updates
  POSITION currentTrippleMixPosition = { 999, 999 };
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

  #pragma region Multi Unit

  std::shared_ptr<INS> unit2;
  std::shared_ptr<INS> unit3;

  #pragma endregion

  #pragma region States

  // Current oven temperature
  double ovenTemperature = 0;
  // Current time in mode
  double timeInMode = 0;
  // Uncorrected time in NAV, starts at AI 5 value, ticks down in align after AI 5, ticks up in NAV
  double initialTimeInNAV = INITIAL_TIME_IN_NAV;
  // Current time in NAV, starts at AI 5 value, ticks down in align after AI 5, ticks up in NAV
  double timeInNAV = INITIAL_TIME_IN_NAV;
  // Time spent in current leg
  double timeInLeg = 0;
  // Time spent in DME update mode
  double timeInDME = 0;
  // Current track
  double track = 0;
  // Cross track distance, positive is plane right of track, nmi
  double crossTrackError = 0;
  // Desired track
  double desiredTrack = 0;
  // Random generators for drift
  std::unique_ptr<std::mt19937> randomGenerator;
  double errorRadial = 0;
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
  // MAKE SURE THIS MATCHES actionMalfunctionCodes.code AT ALL TIMES
  uint8_t displayActionMalfunctionCodeIndex = 0;
  // Leg
  uint8_t currentLegStart = 1;
  uint8_t currentLegEnd = 2;
  // Active DME index, 0 indicates no active DME
  uint8_t activeDME = 0;
  // DME mode
  DME_MODE dmeMode = DME_MODE::INV;
  // Malfunction code displayed if true, action code otherwise
  bool malfunctionCodeDisplayed = false;
  // Test mode
  bool inTestMode = false;
  // HOLD
  bool inHoldMode = false;
  bool holdRequiresForce = false;
  // AUTO/MAN
  bool autoMode = true;
  // If DME is connected
  bool hasDME = false;

  #pragma endregion

  void advanceActionMalfunctionIndex() noexcept;

  void updateSimPosDelta() noexcept;
  void updateCurrentINSPosition(const double dTime) noexcept;
  void updateMetrics(POSITION pos) noexcept;
  void updateNav(POSITION pos, const double dTime) noexcept;
  void updateDisplay(POSITION pos) noexcept;

  void temperatureSim(const double dTime) noexcept;
  void align(const double dTime) noexcept;
  void calculateTrack() noexcept;
  void handleOutOfBounds() noexcept;

  void exportVars() const noexcept;

  void formatActionMalfunctionCode(const bool showingMalf) noexcept;
  void alertLamp(POSITION pos, const double dTime) noexcept;

  void reset(const bool full) noexcept;
  inline void clearDisplay() noexcept {
    uint64_t d;
    d = 0x00CCCCCCCC0CCCCC;
    display = *(reinterpret_cast<DISPLAY *>(&d));
  }

public:
  INS(VarManager &varManager, const std::string &id, const std::string &configID, const std::string &workDir,
      const bool hasDME) noexcept;
  ~INS() noexcept;

  void updatePreMix(const double dTime) noexcept;
  void updatePostMix(const double dTime) noexcept;

  #pragma region Public Getter/Setter

  inline void setDataSelectorPos(const DATA_SELECTOR pos) noexcept {
    dataSelector = pos;
  }
  inline void setModeSelectorPos(const MODE_SELECTOR pos) noexcept {
    modeSelector = pos;
  }
  inline void connectUnit2(std::shared_ptr<INS> &unit) noexcept {
    unit2 = unit;
  }
  inline void connectUnit3(std::shared_ptr<INS> &unit) noexcept {
    unit3 = unit;
  }

  inline DATA_SELECTOR getDataSelectorPos() const noexcept {
    return dataSelector;
  }
  inline MODE_SELECTOR getModeSelectorPos() const noexcept {
    return modeSelector;
  }
  inline bool isAided() const noexcept {
    return activePerformanceIndex == 4;
  }
  inline POSITION getINSPosition() const noexcept {
    return initialINSPosition;
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
  void handleAutoMan() noexcept;
  void handleInstantAlign() noexcept;

  #pragma endregion
};

class INSContainer {
  std::shared_ptr<INS> unit1;
  std::shared_ptr<INS> unit2;
  std::shared_ptr<INS> unit3;

public:
  inline INSContainer(VarManager &varManager, UNIT_COUNT count, UNIT_HAS_DME dme, const std::string &configBaseID) noexcept {
    unit1 = std::make_shared<INS>(varManager, "UNIT_1", "_1", WORK_DIR, dme == UNIT_HAS_DME::ONE || dme == UNIT_HAS_DME::BOTH);

    if (count > UNIT_COUNT::ONE) unit2 = std::make_shared<INS>(varManager, "UNIT_2", configBaseID + "_2", WORK_DIR,
                                                               dme == UNIT_HAS_DME::TWO || dme == UNIT_HAS_DME::BOTH);
    if (count == UNIT_COUNT::THREE) unit3 = std::make_shared<INS>(varManager, "UNIT_3", configBaseID + "_3", WORK_DIR, false);

    if (count > UNIT_COUNT::ONE) unit1->connectUnit2(unit2);
    if (count == UNIT_COUNT::THREE) unit1->connectUnit3(unit3);

    if (count > UNIT_COUNT::ONE) unit2->connectUnit2(unit1);
    if (count == UNIT_COUNT::THREE) unit2->connectUnit3(unit3);

    if (count == UNIT_COUNT::THREE) unit3->connectUnit2(unit1);
    if (count == UNIT_COUNT::THREE) unit3->connectUnit3(unit2);
  }
  inline ~INSContainer() noexcept {
    unit1->~INS();
    if (unit2) unit2->~INS();
    if (unit3) unit3->~INS();
  }

  inline void update(const double dTime) const noexcept {
    unit1->updatePreMix(dTime);
    if (unit2) unit2->updatePreMix(dTime);
    if (unit3) unit3->updatePreMix(dTime);
    // TODO: Mix
    unit1->updatePostMix(dTime);
    if (unit2) unit2->updatePostMix(dTime);
    if (unit3) unit3->updatePostMix(dTime);
  }

  inline void handleEvent(std::function<void(std::shared_ptr<INS>, std::shared_ptr<INS>, std::shared_ptr<INS>)> callback) {
    callback(unit1, unit2, unit3);
  }
};


#endif