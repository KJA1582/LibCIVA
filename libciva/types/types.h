#ifndef TYPES_H
#define TYPES_H

#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdint>

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
  WPT_CHG_FROM,
  WPT_CHG_TO,

};

enum class DME_MODE: int8_t {
  INV = -1,
  DME_LL,
  DME_FREQ,
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

  inline bool isValid() const noexcept {
    return latitude <= 90 && latitude >= -90 && longitude <= 180 && longitude >= -180;
  }

  double distanceTo(const POSITION &target) const noexcept;
  double bearingTo(const POSITION &target) const noexcept;
  // Negative is LEFT
  double crossTrackDistance(const POSITION &start, const POSITION &end) const noexcept;
};

typedef struct {
  POSITION position;
  // In MHz multiplied by 100
  uint16_t frequency;
  // In thousands of feet
  uint8_t altitude;
} DME;

#endif
