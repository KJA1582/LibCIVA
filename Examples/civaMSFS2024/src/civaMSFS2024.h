#ifndef LIBCIVA_MSFS_H
#define LIBCIVA_MSFS_H

#ifndef __INTELLISENSE__
#	define MODULE_EXPORT __attribute__( ( visibility( "default" ) ) )
#	define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#	define MODULE_EXPORT
#	define MODULE_WASM_MODNAME(mod)
#	define __attribute__(x)
#	define __restrict__
#endif

#include <MSFS/Legacy/gauges.h>
#include <MSFS/MSFS.h>
#include <MSFS/MSFS_SystemContext.h>
#include <SimConnect.h>

#include <libciva.h>

#include "varManager.h"

enum DATA_DEFINITIONS: SIMCONNECT_DATA_DEFINITION_ID {
  DATA_DEFINITIONS_DATA,
  DATA_DEFINITIONS_UNIT_1,
  DATA_DEFINITIONS_UNIT_2,
  DATA_DEFINITIONS_UNIT_3,
};

enum REQUEST_DEFINITIONS: SIMCONNECT_DATA_REQUEST_ID {
  REQUEST_DEFINITIONS_DATA
};

struct DATA {
  double airspeedTrue;
  double ambiantTemp;
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

struct EXPORT {
  double displays;
  double indicators;
  double modeSelectorPos;
  double dataSelector;
  double waypointSelectorPos;
  double autoManPos;
  double crossTrackError;
  double desiredTrack;
};

#endif