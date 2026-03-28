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

constexpr uint8_t MIN_MODE_8 = 51;
constexpr uint8_t MAX_MODE_7 = 90;        // Not specified in manual, but "shortly"
constexpr uint16_t MAX_MODE_6 = 420;      // Rest of the ~8.5min MODE 7 and 6
constexpr uint8_t MODE_5_TO_0 = 204;      // 3.4min per mode
constexpr uint16_t TIME_PER_AI = 1200;    // 20min per AI, 3 AI per hour, 3h results in AI9
constexpr uint8_t MAX_BAT_TEST_TIME = 12; // 12s bat test

constexpr uint8_t PROG_NUM[] = {1, 1, 0, 7}; // CIV-A-22

constexpr uint8_t MIN_GS = 75;
constexpr uint8_t MIN_GS_TIME = 10;
constexpr uint8_t MIN_TAS_WIND = 115;
constexpr uint16_t MAX_TAS_WIND = 606;
constexpr uint8_t MAX_RAMP_DEV = 76;
constexpr uint8_t MAX_DEV = 33;
constexpr uint16_t MAX_GS = 910;
constexpr uint16_t DRIFT_GS = 500;
constexpr uint16_t MAX_GS_DISPLAY = 2400;
constexpr uint8_t MAX_DRIFT_ANGLE = 45;
constexpr uint16_t MAX_RADIAL_ERROR_SCALAR_ALIGN_TIME =
    5400; // AI5 Time. This scales the radial error to simulate the difference between full align and minimal align
constexpr double MIN_LEG_TIME = 25.6;
constexpr uint8_t LEG_TIME_ALERT = 120;
constexpr uint8_t MIN_DME_TIME = 12;                 // After this, DME indicator can go green
constexpr uint16_t BATTERY_DURATION = 900;           // Battery runtime, seconds, 15min
constexpr uint16_t EXPANDED_BATTERY_DURATION = 1800; // 30 min
constexpr uint16_t MIN_BATTERY_DURATION = 600;       // 10 min
constexpr uint8_t CHARGE_RATE = 3;                   // 3 seconds of runtime per second gained
constexpr uint8_t MAX_DME_RANGE = 250;               // Maximum range upon which DME updating can occur
constexpr double DME_CORRECTION = 1.0 / 300.0;       // 1nmi per 5 min
constexpr double SINGLE_DME_MIN_ERROR = 0.5;         // nmi, creative license
constexpr double DUAL_DME_MIN_ERROR = 0.0;           // nmi, creative license
constexpr uint16_t DME_AI_TIME = 300;                // 5min
constexpr uint8_t MIX_EASE_TIME = 60;                // Triple mix ease-on-off
constexpr uint8_t ALERT_MIN_GS = 250;                // Minimum GS required for alert lamp to light for leg changes

constexpr uint8_t DISPLAY_CHAR_RIGHT = 10;
constexpr uint8_t DISPLAY_CHAR_LEFT = 11;
constexpr uint8_t DISPLAY_CHAR_BLANK = 12;

constexpr const char *ID_UNIT_1 = "UNIT_1";
constexpr const char *ID_UNIT_2 = "UNIT_2";
constexpr const char *ID_UNIT_3 = "UNIT_3";

class INS {
  friend class INSContainer;

  VarManager &varManager;
  // INS Config
  std::unique_ptr<Config> config;
  // List of active malfunctions
  ACTION_MALFUNCTION_CODES actionMalfunctionCodes;
  // Random
  std::unique_ptr<std::mt19937> randomGen;
  std::unique_ptr<std::normal_distribution<>> distributionRadialDrift;       // °/h 3sigma of 0.01
  std::unique_ptr<std::normal_distribution<>> distributionDistanceDrift;     // nmi/h
  std::unique_ptr<std::normal_distribution<>> distributionSpeedDrift;        // kts/h, additive to a base of 0.1 kts/h
  std::unique_ptr<std::uniform_real_distribution<>> distributionRadialStart; // °, radial error start point

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

  INS *unit2 = NULL;
  INS *unit3 = NULL;

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
  // Ground speed
  double groundSpeed = 0;
  // Current track
  double track = 0;
  // Cross track distance, positive is plane right of track, nmi
  double crossTrackError = 0;
  // Desired track
  double desiredTrack = 0;
  // Track angle Error, positive is TRK < DTK (right turn to align)
  double trackAngleError = 0;
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
  // Speed drift in kts/s, minimum 0.05
  double speedDriftPerSecond = 0;
  // Easing timer
  double mixEaseTime = 0;
  // Alert lamp flash timer
  double flashTime = 0;
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
  // Current performance index
  PERFORMANCE_INDEX activePerformanceIndex = PERFORMANCE_INDEX::UNAIDED;
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
  // Signals valid
  SIGNAL_VALIDITY valid = SIGNAL_VALIDITY::INV;
  // Malfunction code displayed if true, action code otherwise
  bool malfunctionCodeDisplayed = false;
  // Test mode
  bool inTestMode = false;
  // HOLD
  bool inHoldMode = false;
  bool holdRequiresForce = false;
  // AUTO/MAN
  bool autoMode = true;
  // If the waypoint was passed in auto mode
  bool autoModePassed = true;
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

#pragma endregion

#pragma region Unit Index(down here due to memory layout)

  UNIT_INDEX unitIndex;

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

  void remoteUpdateDME(const uint8_t dme, const bool resetDMEUpdate = false) noexcept;
  void remoteUpdateWPT(const uint8_t wpt) noexcept;

  inline void connectUnit2(INS *unit) noexcept { unit2 = unit; }
  inline void connectUnit3(INS *unit) noexcept { unit3 = unit; }

public:
#pragma region Lifecycle

  INS(VarManager &varManager, const UNIT_INDEX id, const std::string &configID, const std::string &workDir, const bool hasADEU,
      const bool hasDME, const bool hasExpandedBattery)
  noexcept;
  ~INS() noexcept;

#pragma endregion

#pragma region Update functions

  void updatePreMix(const double dTime) noexcept;
  void updateMix() noexcept;
  void updatePostMix(const double dTime) noexcept;

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

#pragma region Remote Insert

  // Updating DME in use will drop out of DME updating
  // All DMEs are updated
  void remoteInsertDME(const DME (&dme)[9]) noexcept;

  // Only unused waypoints are updated, if leg is 3-6, then 7,8,9,1,2 are updated
  // If leg is 0-3, 4,5,6,7,8,9 are updated
  // At most 8 waypoints will be imported (2,3,4,5,6,7,8,9)
  void remoteInsertWPT(const POSITION (&wpt)[9]) noexcept;

#pragma endregion
};

} // namespace libciva

#endif