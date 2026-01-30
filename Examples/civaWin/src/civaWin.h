#ifndef CIVA_WIN_H
#define CIVA_WIN_H

#include <windows.h>

#include <iostream>
#include <mutex>
#include <thread>

#include <SimConnect.h>

#include <libciva.h>

#include "varManager.h"

#ifdef __INTELLISENSE__
#define WORK_DIR "./"
#endif

enum DATA_DEFINITIONS : SIMCONNECT_DATA_DEFINITION_ID { DATA_DEFINITIONS_DATA };

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
};

#endif