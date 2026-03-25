#ifndef CIVA_WIN_H
#define CIVA_WIN_H

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

#include <windows.h>

#include <iostream>
#include <memory>
#undef _LIBCPP_HAS_NO_THREADS
#include <mutex>
#include <thread>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/rapidjson.h>

#include <SimConnect.h>

#include <libciva.h>

#include "varManager.h"

// Pure AP Demo
#include "lateralAutopilot.h"
#include "verticalAutopilot.h"

enum DATA_DEFINITIONS : SIMCONNECT_DATA_DEFINITION_ID { DATA_DEFINITIONS_DATA };

enum REQUEST_DEFINITIONS : SIMCONNECT_DATA_REQUEST_ID { REQUEST_DEFINITIONS_DATA };

// Pure AP Demo
enum EVENT_DEFINITIONS : SIMCONNECT_CLIENT_EVENT_ID {
  EVENT_DEFINITIONS_AILERON_SET,
  EVENT_DEFINITIONS_ELEVATOR_SET,
};

struct DATA {
  double airspeedTrue;
  double ambientTemp;
  double windDirection;
  double windSpeed;
  double headingTrue;
  double latitude;
  double longitude;
  double navDME1;
  double navDME2;
  double simRate;
  double altitude;
  double velocityWorldX;
  double velocityWorldZ;
  double accelWorldX;
  double accelWorldZ;

  // Pure AP Demo
  double rollRateBodyZ;
  double planeBankDegrees;
  double pitchRateBodyX;
  double planePitchDegrees;
};

#endif