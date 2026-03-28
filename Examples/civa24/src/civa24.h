#ifndef LIBCIVA_MSFS_H
#define LIBCIVA_MSFS_H

#ifndef __INTELLISENSE__
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include <memory>

#include <MSFS/Legacy/gauges.h>
#include <MSFS/MSFS.h>
#include <MSFS/MSFS_SystemContext.h>
#include <SimConnect.h>

#include <libciva.h>

// Base event IDs
// Unit 2 is +100
// Unit 3 is +200

constexpr const char *EVENT_KEY_0 = 1;
constexpr const char *EVENT_KEY_1 = 2;
constexpr const char *EVENT_KEY_2 = 3;
constexpr const char *EVENT_KEY_3 = 4;
constexpr const char *EVENT_KEY_4 = 5;
constexpr const char *EVENT_KEY_5 = 6;
constexpr const char *EVENT_KEY_6 = 7;
constexpr const char *EVENT_KEY_7 = 8;
constexpr const char *EVENT_KEY_8 = 9;
constexpr const char *EVENT_KEY_9 = 10;
constexpr const char *EVENT_INC_MODE = 11;
constexpr const char *EVENT_DEC_MODE = 12;
constexpr const char *EVENT_INC_DATA = 13;
constexpr const char *EVENT_DEC_DATA = 14;
constexpr const char *EVENT_INC_WPT = 15;
constexpr const char *EVENT_DEC_WPT = 16;
constexpr const char *EVENT_INSERT = 17;
constexpr const char *EVENT_TEST_DOWN = 18;
constexpr const char *EVENT_TEST_UP = 19;
constexpr const char *EVENT_DME_LL = 21;
constexpr const char *EVENT_DME_FREQ = 22;
constexpr const char *EVENT_CLEAR = 23;
constexpr const char *EVENT_WPT_CHG = 24;
constexpr const char *EVENT_HOLD = 25;
constexpr const char *EVENT_AUTO_MAN = 26;
constexpr const char *EVENT_REMOTE = 27;
constexpr const char *EVENT_EXTERNAL_POWER_ON = 28;
constexpr const char *EVENT_EXTERNAL_POWER_OFF = 29;
constexpr const char *EVENT_INSTANT_ALIGN = 30;

enum DATA_DEFINITIONS : SIMCONNECT_DATA_DEFINITION_ID {
  DATA_DEFINITIONS_DATA,
  DATA_DEFINITIONS_EVENT,
  DATA_DEFINITIONS_UNIT_1,
  DATA_DEFINITIONS_UNIT_2,
  DATA_DEFINITIONS_UNIT_3,
};

enum REQUEST_DEFINITIONS : SIMCONNECT_DATA_REQUEST_ID { REQUEST_DEFINITIONS_DATA };

enum EVENT_DEFINITIONS : SIMCONNECT_CLIENT_EVENT_ID {
  EVENT_ID_PAUSE,
};

struct DATA {
  double airspeedTrue;
  double groundVelocity;
  double ambientTemp;
  double windDirection;
  double windSpeed;
  double headingTrue;
  double latitude;
  double longitude;
  double navDME1;
  double navDME2;
  double simRate;
  double event;
  const char atcID[32];
  double altitude;
};

#endif