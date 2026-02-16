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

constexpr auto EVENT_KEY_0 = 1;
constexpr auto EVENT_KEY_1 = 2;
constexpr auto EVENT_KEY_2 = 3;
constexpr auto EVENT_KEY_3 = 4;
constexpr auto EVENT_KEY_4 = 5;
constexpr auto EVENT_KEY_5 = 6;
constexpr auto EVENT_KEY_6 = 7;
constexpr auto EVENT_KEY_7 = 8;
constexpr auto EVENT_KEY_8 = 9;
constexpr auto EVENT_KEY_9 = 10;
constexpr auto EVENT_INC_MODE = 11;
constexpr auto EVENT_DEC_MODE = 12;
constexpr auto EVENT_INC_DATA = 13;
constexpr auto EVENT_DEC_DATA = 14;
constexpr auto EVENT_INC_WPT = 15;
constexpr auto EVENT_DEC_WPT = 16;
constexpr auto EVENT_INSERT = 17;
constexpr auto EVENT_TEST_DOWN = 18;
constexpr auto EVENT_TEST_UP = 19;
constexpr auto EVENT_DME_LL = 21;
constexpr auto EVENT_DME_FREQ = 22;
constexpr auto EVENT_CLEAR = 23;
constexpr auto EVENT_WPT_CHG = 24;
constexpr auto EVENT_HOLD = 25;
constexpr auto EVENT_AUTO_MAN = 26;
constexpr auto EVENT_REMOTE = 27;
constexpr auto EVENT_EXTERNAL_POWER_ON = 28;
constexpr auto EVENT_EXTERNAL_POWER_OFF = 29;
constexpr auto EVENT_INSTANT_ALIGN = 30;

enum DATA_DEFINITIONS : SIMCONNECT_DATA_DEFINITION_ID {
  DATA_DEFINITIONS_DATA,
  DATA_DEFINITIONS_EVENT,
  DATA_DEFINITIONS_UNIT_1,
  DATA_DEFINITIONS_UNIT_2,
  DATA_DEFINITIONS_UNIT_3,
};

enum REQUEST_DEFINITIONS : SIMCONNECT_DATA_REQUEST_ID { REQUEST_DEFINITIONS_DATA };

struct DATA {
  double airspeedTrue;
  double ambientTemp;
  double windDirection;
  double windSpeed;
  double groundSpeed;
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

struct EXPORT {
  double displays;
  double indicators;
  double modeSelectorPos;
  double dataSelectorPos;
  double waypointSelectorPos;
  double autoManPos;
  double crossTrackError;
  double desiredTrack;
  double distance;
  double valid;
};

#endif