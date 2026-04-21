#ifndef TYPES_H
#define TYPES_H

// For MSFS so IntelliSense doesn't crap out
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

#include <cmath>
#include <cstdint>
#include <iostream>

namespace libciva {

static constexpr double PI = 3.14159265358979323846;
static constexpr double EPSILON = 2.22044604925031308085e-16;
static constexpr double DEG2RAD = PI / 180.0;
static constexpr double RAD2DEG = 180.0 / PI;

enum class UNIT_COUNT : uint8_t {
  ONE,
  TWO,
  THREE,
};

enum class UNIT_HAS_DME : uint8_t {
  NONE,
  ONE,
  TWO,
  BOTH,
};

enum class UNIT_INDEX : uint8_t { UNIT_1, UNIT_2, UNIT_3 };

enum class PERFORMANCE_INDEX : uint8_t {
  ERADICATE = 1,
  AIDED = 4,
  UNAIDED = 5,
};

enum class DATA_SELECTOR : uint8_t { TKGS, HDGDA, XTKTKE, POS, WPT, DISTIME, WIND, DSRTKSTS };

enum class MODE_SELECTOR : uint8_t {
  OFF,
  STBY,
  ALIGN,
  NAV,
  ATT,
};

enum class INS_STATE : uint8_t {
  OFF,
  STBY,
  ALIGN,
  NAV,
  ATT,
  FAIL,
};

enum class ALIGN_SUBMODE : uint8_t {
  MODE_0, // See MODE_4, minimum
  MODE_1, // See MODE_4
  MODE_2, // See MODE_4
  MODE_3, // See MODE_4
  MODE_4, // 3.4min from 4 to 3 etc.
  MODE_5, // Directly at end of MODE_6
  MODE_6, // See MODE_7
  MODE_7, // together with MODE_6, ~8.5min if pres pos was entered before MODE_7 finished
  MODE_8, // min 51s, BAT test
  MODE_9, // STBY, change to 8 if ALIGN entered and warmed up
};

enum class BATTERY_TEST : uint8_t {
  IDLE,
  RUNNING,
  COMPLETED,
  FAILED,
  INHIBITED,
};

enum class INSERT_MODE : int8_t {
  INV = -1,
  POS_LAT,
  PRE_POS_LON,
  POS_LON,
  WPT_LAT,
  WPT_LON,
  DME_FREQ,
  DME_ALT,
  PERFORMANCE_INDEX,
  WPT_CHG_FROM,
  WPT_CHG_TO,
};

enum class DME_MODE : int8_t {
  INV = -1,
  DME_LL,
  DME_FREQ,
};

enum class SIGNAL_VALIDITY : uint8_t {
  INV,
  ATT,
  NAV,
};

// Use Bitset to set
// Use value to unset/check if INV
// Use ENUM of same name to work with
typedef union {
  uint64_t value; // INV = 0,
  struct {
    bool A02_31 : 1; // 1 ground speed > 910, non-clearable
    bool A02_42 : 1; // 2 drift angle > 45, non-clearable
    bool A02_49 : 1; // 3 pos update in flight > 33, clearable in NAV
    bool A02_63 : 1; // 4 self checks failed, non-clearable
    bool A04_41 : 1; // 5 ramp pos > 76nmi from last pos, clearable in STBY
    bool A04_43 : 1; // 6 ramp pos mismatch between pairs of units, clearable in STBY
    bool A04_57 : 1; // 7 taxi during align, clearable in STBY
  } codes;
} ACTION_MALFUNCTION_CODES;

// Use Bitset to set/reset indicators
typedef union {
  uint32_t value;
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
    bool FROM_BLINK : 1;
    bool TO_BLINK : 1;
    bool DME1 : 1;
    bool DME2 : 1;
  } indicator;
} INDICATORS;

// Use bitset to set display characters
// Characters are encoded 0->9 for 0->9, 10 for R, 11 for L, 12 for " "
// TO/FROM values
typedef union {
  uint64_t value;
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
    uint8_t FROM : 4;
    uint8_t TO : 4;
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

struct POSITION {
  // In degrees
  double latitude;
  // In degrees
  double longitude;

  inline bool isValid() const noexcept { return latitude <= 90 && latitude >= -90 && longitude <= 180 && longitude >= -180; }

  double distanceTo(const POSITION &target) const noexcept;
  double bearingTo(const POSITION &target) const noexcept;
  // Negative is LEFT
  double crossTrackDistance(const POSITION &start, const POSITION &end) const noexcept;
  double alongTrackDistance(const POSITION &start, const POSITION &end) const noexcept;
  POSITION destination(const double distance, const double bearing) const noexcept;
  void bound() noexcept;
  bool inFront(const POSITION &pos, const double track) const noexcept;
  POSITION intersection(const double bearing1, const POSITION &pos2, const double bearing2) noexcept;

  inline bool operator==(const POSITION &rhs) const noexcept {
    return latitude - rhs.latitude < 0.1 && longitude - rhs.longitude < 0.1;
  }
  inline bool operator!=(const POSITION &rhs) const noexcept { return !(*this == rhs); }

  // Not bound checked
  inline POSITION operator+(const POSITION &rhs) const noexcept { return {latitude + rhs.latitude, longitude + rhs.longitude}; }
  // Not bound checked
  inline POSITION operator-(const POSITION &rhs) const noexcept { return {latitude - rhs.latitude, longitude - rhs.longitude}; }
  // Not bound checked
  inline POSITION operator*(double scalar) const noexcept { return {latitude * scalar, longitude * scalar}; }
  // Not bound checked
  inline POSITION operator/(double scalar) const noexcept { return {latitude / scalar, longitude / scalar}; }

  friend std::ostream &operator<<(std::ostream &os, const POSITION &dt);
};

struct DME {
  POSITION position;
  // In MHz multiplied by 100
  uint16_t frequency;
  // In thousands of feet
  uint8_t altitude;
};

#pragma region Helpers

static inline double absDeltaAngle(const double x, const double y) noexcept {
  return 180.0 - std::fabs(std::fmod(std::fabs(x - y), 360.0) - 180.0);
}

static inline double deltaAngle(const double x, const double y) noexcept {
  double delta = x - y;
  if (delta > 180.0) delta -= 360.0;
  else if (delta <= -180.0)
    delta += 360.0;
  return delta;
}

#pragma endregion

} // namespace libciva

#endif
