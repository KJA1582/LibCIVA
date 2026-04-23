#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#ifndef __INTELLISENSE__
#ifndef MODULE_EXPORT
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#endif
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include <cstdint>
#include <vector>

#include "../types/types.h"

namespace libciva {

class Snapshot {
public:
  static constexpr uint32_t LIBCIVA_SNAPSHOT_VERSION = 1;

#pragma pack(push, 1)
  struct UnitData {
    uint8_t state;
    uint8_t alignSubmode;
    uint8_t batteryTest;
    uint8_t dataSelector;
    uint8_t modeSelector;
    int8_t insertMode;
    uint8_t activePerformanceIndex;
    uint8_t accuracyIndex;
    uint8_t charactersRead;
    uint8_t waypointSelector;
    uint8_t displayActionMalfunctionCodeIndex;
    uint8_t currentLegStart;
    uint8_t currentLegEnd;
    uint8_t activeDME;
    int8_t dmeMode;
    uint8_t valid;
    uint8_t actionMalfunctionCodes;
    uint32_t indicators;
    uint32_t previousIndicators;

    POSITION displayPosition;
    POSITION initialINSPosition;
    POSITION currentINSPosition;
    POSITION currentTripleMixPosition;
    POSITION holdINSPosition;
    POSITION holdPosition;
    POSITION simPosDelta;
    POSITION waypoints[10];
    DME dmes[9];

    uint64_t display;
    uint64_t flags;

    double batteryRuntime;
    double ovenTemperature;
    double timeInMode;
    double radialScalarAlignTime;
    double timeInLeg;
    double timeInDME;
    double groundSpeed;
    double track;
    double crossTrackError;
    double desiredTrack;
    double trackAngleError;
    double remainingDistance;
    double initialDistanceError;
    double currentDistanceError;
    double initialRadialError;
    double currentRadialError;
    double radialDriftPerSecond;
    double baseRadialDriftPerSecond;
    double distanceDriftPerSecond;
    double speedDriftPerSecond;
    double mixEaseTimer;
    double flashTimer;
    double displayTimer;
  };
#pragma pack(pop)

  static constexpr uint8_t FLAG_MALFUNCTION_CODE_DISPLAYED = 0;
  static constexpr uint8_t FLAG_IN_TEST_MODE = 1;
  static constexpr uint8_t FLAG_IN_HOLD_MODE = 2;
  static constexpr uint8_t FLAG_HOLD_REQUIRES_FORCE = 3;
  static constexpr uint8_t FLAG_AUTO_MODE = 4;
  static constexpr uint8_t FLAG_AUTO_MODE_PASSED = 5;
  static constexpr uint8_t FLAG_HAS_FACED_TO_WAYPOINT = 6;
  static constexpr uint8_t FLAG_HAS_ADEU = 7;
  static constexpr uint8_t FLAG_HAS_DME = 8;
  static constexpr uint8_t FLAG_DME_ARMED = 9;
  static constexpr uint8_t FLAG_DME_UPDATING = 10;
  static constexpr uint8_t FLAG_REMOTE_ACTIVE = 11;
  static constexpr uint8_t FLAG_HAS_EXPANDED_BATTERY = 12;
  static constexpr uint8_t FLAG_EXTERNAL_POWER = 13;

  static constexpr size_t UNIT_DATA_SIZE = sizeof(UnitData);

  uint32_t version = LIBCIVA_SNAPSHOT_VERSION;
  uint8_t unitCount = 0;
  std::vector<UnitData> units;

  std::vector<uint8_t> serialize() const noexcept;
  bool deserialize(const std::vector<uint8_t> &data) noexcept;
};

} // namespace libciva

#endif