#ifndef CIVA_WIN_H
#define CIVA_WIN_H

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

// HACK
#ifndef CIVA_WIN
#include <__config>
#undef _LIBCPP_HAS_NO_THREADS
#endif
#endif

#include <windows.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/rapidjson.h>

#pragma warning(push)
#pragma warning(disable : 4245)
#include <SimConnect.h>
#pragma warning(pop)

#include <libciva.h>

#include "varManager.h"

// Pure AP Demo
#include "lateralAutopilot.h"
#include "verticalAutopilot.h"

enum DATA_DEFINITIONS : SIMCONNECT_DATA_DEFINITION_ID { DATA_DEFINITIONS_DATA };

enum REQUEST_DEFINITIONS : SIMCONNECT_DATA_REQUEST_ID { REQUEST_DEFINITIONS_DATA };

enum EVENT_DEFINITIONS : SIMCONNECT_CLIENT_EVENT_ID {
  EVENT_DEFINITIONS_PAUSE,
  // Pure AP Demo
  EVENT_DEFINITIONS_AILERON_SET,
  EVENT_DEFINITIONS_ELEVATOR_SET,
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
  double altitude;

  // Pure AP Demo
  double rollRateBodyZ;
  double planeBankDegrees;
  double pitchRateBodyX;
  double planePitchDegrees;
};

#endif