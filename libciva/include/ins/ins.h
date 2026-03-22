#ifndef INS_H
#define INS_H

// For MSFS so IntelliSense doesn't crap out
#ifndef __INTELLISENSE__
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "config/config.h"
#include "logger/logger.h"
#include "types/types.h"
#include "varManager/varManager.h"

namespace libciva {

constexpr double OPERATING_TEMP = 76;      // °C
constexpr double HEATER_WATTAGE = 2000;    // W
constexpr double HEATER_EFFICIENCY = 0.9;  // %
constexpr double UNIT_MASS = 5;            // kg
constexpr double UNIT_SPECIFIC_HEAT = 900; // J/(kg * K)

constexpr double MAX_BANK_ANGLE = 30; // °
constexpr double MAX_BANK_RATE = 10;  // °/s
constexpr double LEAD_CORRECTION = (MAX_BANK_ANGLE / MAX_BANK_RATE) / 3600.0;

constexpr auto MIN_MODE_8 = 51;
constexpr auto MAX_MODE_7 = 90;        // Not specified in manual, but "shortly"
constexpr auto MAX_MODE_6 = 420;       // Rest of the ~8.5min MODE 7 and 6
constexpr auto MODE_5_TO_0 = 204;      // 3.4min per mode
constexpr auto TIME_PER_AI = 1200;     // 20min per AI, 3 AI per hour, 3h results in AI9
constexpr auto MAX_BAT_TEST_TIME = 12; // 12s bat test

constexpr uint8_t PROG_NUM[] = {1, 1, 0, 7}; // CIV-A-22

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
constexpr auto MAX_RADIAL_ERROR_SCALAR_ALIGN_TIME =
    5400; // AI5 Time. This scales the radial error to simulate the difference between full align and minimal align
constexpr auto MIN_LEG_TIME = 25.6;
constexpr auto LEG_TIME_ALERT = 120;
constexpr auto MIN_DME_TIME = 12;                // After this, DME indicator can go green
constexpr auto BATTERY_DURATION = 900;           // Battery runtime, seconds, 15min
constexpr auto EXPANDED_BATTERY_DURATION = 1800; // 30 min
constexpr auto MIN_BATTERY_DURATION = 600;       // 10 min
constexpr auto CHARGE_RATE = 3;                  // 3 seconds of runtime per second gained
constexpr auto MAX_DME_RANGE = 250;              // Maximum range upon which DME updating can occur
constexpr auto DME_CORRECTION = 1.0 / 300.0;     // 1nmi per 5 min
constexpr auto DME_AI_TIME = 300;                // 5min
constexpr auto MIX_EASE_TIME = 60;               // Triple mix ease-on-off

constexpr auto DISPLAY_CHAR_RIGHT = 10;
constexpr auto DISPLAY_CHAR_LEFT = 11;
constexpr auto DISPLAY_CHAR_BLANK = 12;

constexpr auto ID_UNIT_1 = "UNIT_1";
constexpr auto ID_UNIT_2 = "UNIT_2";
constexpr auto ID_UNIT_3 = "UNIT_3";

class INS {
  VarManager &varManager;
  uint8_t unitIndex; // 0 to 2, corresponding to units 1 to 3
  // INS Config
  std::unique_ptr<Config> config;
  // List of active malfunctions
  ACTION_MALFUNCTION_CODES actionMalfunctionCodes;
  // Random
  std::unique_ptr<std::mt19937> randomGen;
  std::unique_ptr<std::normal_distribution<>> distributionRadial;   // °/h 3sigma of 0.01
  std::unique_ptr<std::normal_distribution<>> distributionDistance; // nmi/h

#pragma region Positions

  // Current position displayed on unit
  POSITION displayPosition = {999, 999};
  // INS Position without any updates
  POSITION initialINSPosition = {999, 999};
  // Current INS position with updates
  POSITION currentINSPosition = {999, 999};
  // Current INS triple mix position with updates
  POSITION currentTripleMixPosition = {999, 999};
  // HOLD mode
  POSITION holdINSPosition = {999, 999};
  POSITION holdPosition = {999, 999};
  // Delta to actual sim position
  POSITION simPosDelta = {0, 0};

#pragma endregion

#pragma region Waypoints / DMEs

  // Waypoints (0 is not settable by hand)
  POSITION waypoints[10] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
  // DMEs, index of selector - 1 = array index
  DME DMEs[9] = {{{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0},
                 {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}};

#pragma endregion

#pragma region Indicators / Display

  // Active indicators
  INDICATORS indicators = {0};
  // Previous indicators for test mode
  INDICATORS previousIndicators = {0};
  // Current Display
  DISPLAY display = {0};

#pragma endregion

#pragma region Multi Unit

  INS *unit2;
  INS *unit3;

#pragma endregion

#pragma region States

  // Battery runtime left
  double batteryRuntime = BATTERY_DURATION;
  // Current oven temperature
  double ovenTemperature = 0;
  // Current time in mode
  double timeInMode = 0;
  // Starts at AI 5 value, ticks down in align after AI 5, used to scale radial error
  double radialScalarAlignTime = MAX_RADIAL_ERROR_SCALAR_ALIGN_TIME;
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
  // Remaining distance to waypoint (from along pos)
  double remainingDistance = 0;
  // Initial INS accumulated distance error
  double initialDistanceError = 0;
  // Current INS position accumulated distance error
  double currentDistanceError = 0;
  // Initial INS accumulated radial error
  double initialRadialError = 0;
  // Current INS accumulated radial error
  double currentRadialError = 0;
  // Radial drift in °/s
  double radialDriftPerSecond = 0;
  double baseRadialDriftPerSecond = 0;
  // Distance drift in nmi/s
  double distanceDriftPerSecond = 0;
  // Easing timer
  double mixEaseTime = 0;
  // Current INS State
  INS_STATE state = INS_STATE::OFF;
  // Current align submode
  ALIGN_SUBMODE alignSubmode = ALIGN_SUBMODE::MODE_9;
  // State of battery test
  BATTERY_TEST batteryTest = BATTERY_TEST::IDLE;
  // Position of data selector
  DATA_SELECTOR dataSelector = {DATA_SELECTOR::POS};
  // Position of mode selector
  MODE_SELECTOR modeSelector = {MODE_SELECTOR::OFF};
  // Current active insert mode
  INSERT_MODE insertMode = {INSERT_MODE::INV};
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
  // If ADEU is connected
  bool hasADEU = false;
  // If DME is connected
  bool hasDME = false;
  // If DME is armed
  bool dmeArmed = false;
  // If DME is updating
  bool dmeUpdating = false;
  // Remote active
  bool remoteActive = false;
  // If battery is expanded
  bool hasExpandedBattery = false;
  // External power state
  bool externalPower = false;
  // Signals valid
  bool valid = false;

#pragma endregion

  void advanceActionMalfunctionIndex() noexcept;
  const POSITION currentNavPosition(const double dTime) noexcept;

  void updateSimPosDelta() noexcept;
  void updateCurrentINSPosition(const double dTime) noexcept;
  void updateMetrics(const double dTime) noexcept;
  void updateNav(const double dTime) noexcept;
  void updateDisplay(const double dTime) noexcept;

  void temperatureBatterySim(const double dTime) noexcept;
  void calculateTrack() noexcept;
  void formatActionMalfunctionCode(const bool showingMalf) noexcept;
  void alertLamp(const double dTime) noexcept;

  void align(const double dTime) noexcept;

  void handleOutOfBounds() noexcept;
  void dmeUpdateChecks(const double dTime) noexcept;

  void exportVars() const noexcept;

  void reset(const bool full) noexcept;
  inline void clearDisplay() noexcept {
    uint64_t d;
    d = 0x00CCCCCCCC0CCCCC;
    display = *(reinterpret_cast<DISPLAY *>(&d));
  }

public:
#pragma region Lifecycle

  INS(VarManager &varManager, const uint8_t id, const std::string &configID, const std::string &workDir, const bool hasADEU,
      const bool hasDME, const bool hasExpandedBattery)
  noexcept;
  ~INS() noexcept;

#pragma endregion

#pragma region Update functions

  void updatePreMix(const double dTime) noexcept;
  void updateMix() noexcept;
  void updatePostMix(const double dTime) noexcept;

#pragma endregion

#pragma region Public Getter / Setter

  inline void connectUnit2(INS *unit) noexcept { unit2 = unit; }
  inline void connectUnit3(INS *unit) noexcept { unit3 = unit; }

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
  void handleRemote() noexcept;
  void handleExternalPower(const bool powered) noexcept;

#pragma endregion

#pragma region Remote Update / Insert

  void remoteUpdateDME(const uint8_t dme, const bool resetDMEUpdate = false) noexcept;
  void remoteUpdateWPT(const uint8_t wpt) noexcept;

  // DMEs are *moved*
  // Updating DME in use will drop out of DME updating
  // All DMEs are updated
  void remoteInsertDME(const DME dme[9]) noexcept;

  // WPTs are *moved*
  // Only unused waypoints are updated, if leg is 3-6, then 7,8,9,1,2 are updated
  // If leg is 0-3, 4,5,6,7,8,9 are updated
  // At most 8 waypoints will be imported (2,3,4,5,6,7,8,9)
  void remoteInsertWPT(const POSITION wpt[9]) noexcept;

#pragma endregion
};

class INSContainer {
  std::shared_ptr<INS> unit1;
  std::shared_ptr<INS> unit2;
  std::shared_ptr<INS> unit3;

public:
  inline INSContainer(VarManager &varManager, UNIT_COUNT count, UNIT_HAS_DME dme, const std::string &configBaseID,
                      const bool hasADEU, const bool hasExtendedBattery) noexcept {
    unit1 = std::make_shared<INS>(varManager, 0, configBaseID + "_1", WORK_DIR,
                                  dme == UNIT_HAS_DME::ONE || dme == UNIT_HAS_DME::BOTH, hasADEU, hasExtendedBattery);

    if (count > UNIT_COUNT::ONE)
      unit2 = std::make_shared<INS>(varManager, 1, configBaseID + "_2", WORK_DIR,
                                    dme == UNIT_HAS_DME::TWO || dme == UNIT_HAS_DME::BOTH, hasADEU, hasExtendedBattery);
    if (count == UNIT_COUNT::THREE)
      unit3 = std::make_shared<INS>(varManager, 2, configBaseID + "_3", WORK_DIR, false, hasADEU, hasExtendedBattery);

    if (count > UNIT_COUNT::ONE) unit1->connectUnit2(unit2.get());
    if (count == UNIT_COUNT::THREE) unit1->connectUnit3(unit3.get());

    if (count > UNIT_COUNT::ONE) unit2->connectUnit2(unit1.get());
    if (count == UNIT_COUNT::THREE) unit2->connectUnit3(unit3.get());

    if (count == UNIT_COUNT::THREE) unit3->connectUnit2(unit1.get());
    if (count == UNIT_COUNT::THREE) unit3->connectUnit3(unit2.get());
  }

  void update(const double dTime) const noexcept;

  inline void handleEvent(std::function<void(std::shared_ptr<INS>, std::shared_ptr<INS>, std::shared_ptr<INS>)> callback) {
    callback(unit1, unit2, unit3);
  }
};

} // namespace libciva

#endif