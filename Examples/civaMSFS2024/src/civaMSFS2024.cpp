#include "civaMSFS2024.h"

std::unique_ptr<MSFSVarManager> varManager;
std::unique_ptr<INSContainer> ins;

#pragma region SimConnect

HANDLE simConnect = 0;

static void setupSimConnect() {
  HRESULT hr;

  hr = SimConnect_Open(&simConnect, "libciva", NULL, 0, NULL, 0);
  if (FAILED(hr)) return;

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_AIRSPEED_TRUE, "KNOT");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_AMBIENT_TEMPERATURE, "CELSIUS");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_AMBIENT_WIND_DIRECTION, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_AMBIENT_WIND_VELOCITY, "KNOT");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_GROUND_VELOCITY, "KNOT");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_PLANE_HEADING_DEGREES_TRUE, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_PLANE_LATITUDE, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_PLANE_LONGITUDE, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_NAV_DME_1, "NAUTICAL MILE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_NAV_DME_2, "NAUTICAL MILE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, SIM_VAR_SIMULATION_RATE, "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + DISPLAY_VAR + "UNIT_1").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + INDICATORS_VAR + "UNIT_1").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + MODE_SELECTOR_POS_VAR + "UNIT_1").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + DATA_SELECTOR_POS_VAR + "UNIT_1").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + WAYPOINT_SELECTOR_POS_VAR + "UNIT_1").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + AUTO_MAN_POS_VAR + "UNIT_1").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + OUTPUT_CROSS_TRACK_ERROR + "UNIT_1").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1,
                                 (std::string("L:") + VAR_START + OUTPUT_DESIRED_TRACK + "UNIT_1").c_str(), "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + DISPLAY_VAR + "UNIT_2").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + INDICATORS_VAR + "UNIT_2").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + MODE_SELECTOR_POS_VAR + "UNIT_2").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + DATA_SELECTOR_POS_VAR + "UNIT_2").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + WAYPOINT_SELECTOR_POS_VAR + "UNIT_2").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + AUTO_MAN_POS_VAR + "UNIT_2").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + OUTPUT_CROSS_TRACK_ERROR + "UNIT_2").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2,
                                 (std::string("L:") + VAR_START + OUTPUT_DESIRED_TRACK + "UNIT_2").c_str(), "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + DISPLAY_VAR + "UNIT_3").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + INDICATORS_VAR + "UNIT_3").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + MODE_SELECTOR_POS_VAR + "UNIT_3").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + DATA_SELECTOR_POS_VAR + "UNIT_3").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + WAYPOINT_SELECTOR_POS_VAR + "UNIT_3").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + AUTO_MAN_POS_VAR + "UNIT_3").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + OUTPUT_CROSS_TRACK_ERROR + "UNIT_3").c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3,
                                 (std::string("L:") + VAR_START + OUTPUT_DESIRED_TRACK + "UNIT_3").c_str(), "NUMBER");

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA,
                                    SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_VISUAL_FRAME);
}

static void handleSimConnect() {
  SIMCONNECT_RECV *pData;
  DWORD cbData;

  while (true) {
    HRESULT hr = SimConnect_GetNextDispatch(simConnect, &pData, &cbData);
    if (SUCCEEDED(hr)) {
      switch (pData->dwID) {
        case SIMCONNECT_RECV_ID_NULL:
          return;

        case SIMCONNECT_RECV_ID_OPEN: {
          SIMCONNECT_RECV_OPEN *openData = (SIMCONNECT_RECV_OPEN *)pData;
          Logger::GetInstance() << "Connected to " << openData->szApplicationName << "\n";
          break;
        }
        case SIMCONNECT_RECV_ID_EVENT: {
          SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT *)pData;
          break;
        }
        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
          SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA *)pData;
          DATA *data = (DATA *)&pObjData->dwData;
          varManager->setVar(SIM_VAR_AIRSPEED_TRUE, data->airspeedTrue);
          varManager->setVar(SIM_VAR_AMBIENT_TEMPERATURE, data->ambiantTemp);
          varManager->setVar(SIM_VAR_AMBIENT_WIND_DIRECTION, data->windDirection);
          varManager->setVar(SIM_VAR_AMBIENT_WIND_VELOCITY, data->windSpeed);
          varManager->setVar(SIM_VAR_GROUND_VELOCITY, data->groundSpeed);
          varManager->setVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, data->headingTrue);
          varManager->setVar(SIM_VAR_PLANE_LATITUDE, data->latitude);
          varManager->setVar(SIM_VAR_PLANE_LONGITUDE, data->longitude);
          varManager->setVar(SIM_VAR_NAV_DME_1, data->navDME1);
          varManager->setVar(SIM_VAR_NAV_DME_2, data->navDME2);
          varManager->setVar(SIM_VAR_SIMULATION_RATE, data->simRate);
          break;
        }
        case SIMCONNECT_RECV_ID_EXCEPTION: {
          SIMCONNECT_RECV_EXCEPTION *except = (SIMCONNECT_RECV_EXCEPTION *)pData;
          Logger::GetInstance() << "SimConnect Exception: " << except->dwException << "\n";
          break;
        }
        default:
          break;
      }
    }
  }
}

#pragma endregion

extern "C" MSFS_CALLBACK void module_init(void) {
  varManager = std::make_unique<MSFSVarManager>();

  ins = std::make_unique<INSContainer>(*varManager, UNIT_COUNT::THREE, UNIT_HAS_DME::BOTH, "");

  setupSimConnect();
}

extern "C" MSFS_CALLBACK bool libciva_gauge_update(FsContext ctx, float dTime) {
  // This is called each frame
  // dTime is the time between the previous frame and this one
  // return false if there is any error

  handleSimConnect();

  double simRate = 1;
  varManager->getVar(SIM_VAR_SIMULATION_RATE, simRate);
  ins->update(dTime * 1e-7 * simRate);

  return true;
}

extern "C" MSFS_CALLBACK void module_deinit(void) {
  // This is called when the module is deinitialized

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA,
                                    SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_NEVER);
}
