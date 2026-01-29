#include "civaMSFS2024.h"

std::unique_ptr<VarManager> varManager;
std::unique_ptr<INSContainer> ins;

#pragma region Preset LVAR names

std::string DISPLAY_VAR_UNIT_1 =  std::string("L") + DISPLAY_VAR + "UNIT_1";
std::string INDICATORS_VAR_UNIT_1 = std::string("L") + INDICATORS_VAR + "UNIT_1";
std::string MODE_SELECTOR_POS_VAR_UNIT_1 = std::string("L") + MODE_SELECTOR_POS_VAR + "UNIT_1";
std::string DATA_SELECTOR_POS_VAR_UNIT_1 = std::string("L") + DATA_SELECTOR_POS_VAR + "UNIT_1";
std::string WAYPOINT_SELECTOR_POS_VAR_UNIT_1 = std::string("L") + WAYPOINT_SELECTOR_POS_VAR + "UNIT_1";
std::string AUTO_MAN_POS_VAR_UNIT_1 = std::string("L") + AUTO_MAN_POS_VAR + "UNIT_1";
std::string CROSS_TRACK_ERROR_VAR_UNIT_1 = std::string("L") + CROSS_TRACK_ERROR_VAR + "UNIT_1";
std::string DESIRED_TRACK_VAR_UNIT_1 = std::string("L") + DESIRED_TRACK_VAR + "UNIT_1";

std::string DISPLAY_VAR_UNIT_2 = std::string("L") + DISPLAY_VAR + "UNIT_2";
std::string INDICATORS_VAR_UNIT_2 = std::string("L") + INDICATORS_VAR + "UNIT_2";
std::string MODE_SELECTOR_POS_VAR_UNIT_2 = std::string("L") + MODE_SELECTOR_POS_VAR + "UNIT_2";
std::string DATA_SELECTOR_POS_VAR_UNIT_2 = std::string("L") + DATA_SELECTOR_POS_VAR + "UNIT_2";
std::string WAYPOINT_SELECTOR_POS_VAR_UNIT_2 = std::string("L") + WAYPOINT_SELECTOR_POS_VAR + "UNIT_2";
std::string AUTO_MAN_POS_VAR_UNIT_2 = std::string("L") + AUTO_MAN_POS_VAR + "UNIT_2";
std::string CROSS_TRACK_ERROR_VAR_UNIT_2 = std::string("L") + CROSS_TRACK_ERROR_VAR + "UNIT_2";
std::string DESIRED_TRACK_VAR_UNIT_2 = std::string("L") + DESIRED_TRACK_VAR + "UNIT_2";

std::string DISPLAY_VAR_UNIT_3 = std::string("L") + DISPLAY_VAR + "UNIT_3";
std::string INDICATORS_VAR_UNIT_3 = std::string("L") + INDICATORS_VAR + "UNIT_3";
std::string MODE_SELECTOR_POS_VAR_UNIT_3 = std::string("L") + MODE_SELECTOR_POS_VAR + "UNIT_3";
std::string DATA_SELECTOR_POS_VAR_UNIT_3 = std::string("L") + DATA_SELECTOR_POS_VAR + "UNIT_3";
std::string WAYPOINT_SELECTOR_POS_VAR_UNIT_3 = std::string("L") + WAYPOINT_SELECTOR_POS_VAR + "UNIT_3";
std::string AUTO_MAN_POS_VAR_UNIT_3 = std::string("L") + AUTO_MAN_POS_VAR + "UNIT_3";
std::string CROSS_TRACK_ERROR_VAR_UNIT_3 = std::string("L") + CROSS_TRACK_ERROR_VAR + "UNIT_3";
std::string DESIRED_TRACK_VAR_UNIT_3 = std::string("L") + DESIRED_TRACK_VAR + "UNIT_3";

#pragma endregion

#pragma region Events

static void handleEvent(double event) {
  ins->handleEvent([event](auto unit1, auto unit2, auto unit3) {
    switch((int)event) {
      case EVENT_KEY_0:
      case EVENT_KEY_1:
      case EVENT_KEY_2:
      case EVENT_KEY_3:
      case EVENT_KEY_4:
      case EVENT_KEY_5:
      case EVENT_KEY_6:
      case EVENT_KEY_7:
      case EVENT_KEY_8:
      case EVENT_KEY_9:
        unit1->handleNumeric(event - 1);
        if (unit2) unit2->handleNumeric(event - 1);
        if (unit3) unit3->handleNumeric(event - 1);
        break;
      case EVENT_INC_MODE:
        unit1->decModeSelectorPos();
        if (unit2) unit2->incModeSelectorPos();
        if (unit3) unit3->incModeSelectorPos();
        break;
      case EVENT_DEC_MODE:
        unit1->decModeSelectorPos();
        if (unit2) unit2->decModeSelectorPos();
        if (unit3) unit3->decModeSelectorPos();
        break;
      case EVENT_INC_DATA:
        unit1->incDataSelectorPos();
        if (unit2) unit2->incDataSelectorPos();
        if (unit3) unit3->incDataSelectorPos();
        break;
      case EVENT_DEC_DATA:
        unit1->incDataSelectorPos();
        if (unit2) unit2->decDataSelectorPos();
        if (unit3) unit3->decDataSelectorPos();
        break;
      case EVENT_INC_WPT:
        unit1->incWaypointSelectorPos();
        if (unit2) unit2->incWaypointSelectorPos();
        if (unit3) unit3->incWaypointSelectorPos();
        break;
      case EVENT_DEC_WPT:
        unit1->incWaypointSelectorPos();
        if (unit2) unit2->decWaypointSelectorPos();
        if (unit3) unit3->decWaypointSelectorPos();
        break;
      case EVENT_INSERT:
        unit1->handleInsert();
        if (unit2) unit2->handleInsert();
        if (unit3) unit3->handleInsert();
        break;
      case EVENT_TEST_DOWN:
        unit1->handleTestButtonState(true);
        if (unit2) unit2->handleTestButtonState(true);
        if (unit3) unit3->handleTestButtonState(true);
        break;
      case EVENT_TEST_UP:
        unit1->handleTestButtonState(false);
        if (unit2) unit2->handleTestButtonState(false);
        if (unit3) unit3->handleTestButtonState(false);
        break;
      case EVENT_DME_LL:
        unit1->handleDMEModeEntry('L');
        if (unit2) unit2->handleDMEModeEntry('L');
        if (unit3) unit3->handleDMEModeEntry('L');
        break;
      case EVENT_DME_FREQ:
        unit1->handleDMEModeEntry('F');
        if (unit2) unit2->handleDMEModeEntry('F');
        if (unit3) unit3->handleDMEModeEntry('F');
        break;
      case EVENT_CLEAR:
        unit1->handleClear();
        if (unit2) unit2->handleClear();
        if (unit3) unit3->handleClear();
        break;
      case EVENT_WPT_CHG:
        unit1->handleWaypointChange();
        if (unit2) unit2->handleWaypointChange();
        if (unit3) unit3->handleWaypointChange();
        break;
      case EVENT_HOLD:
        unit1->handleHoldButton();
        if (unit2) unit2->handleHoldButton();
        if (unit3) unit3->handleHoldButton();
        break;
      case EVENT_AUTO_MAN:
        unit1->handleAutoMan();
        if (unit2) unit2->handleAutoMan();
        if (unit3) unit3->handleAutoMan();
        break;
      case EVENT_INSTANT_ALIGN:
        unit1->handleInstantAlign();
        if (unit2) unit2->handleInstantAlign();
        if (unit3) unit3->handleInstantAlign();
        break;
      default:
        Logger::GetInstance() << "Unknown event " << event;
    }
  });
}

#pragma endregion

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
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, EVENT, "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, DISPLAY_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, INDICATORS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, MODE_SELECTOR_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, DATA_SELECTOR_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, WAYPOINT_SELECTOR_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, AUTO_MAN_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, CROSS_TRACK_ERROR_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, DESIRED_TRACK_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, EVENT, "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, DISPLAY_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, INDICATORS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, MODE_SELECTOR_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, DATA_SELECTOR_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, WAYPOINT_SELECTOR_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, AUTO_MAN_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, CROSS_TRACK_ERROR_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, DESIRED_TRACK_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, EVENT, "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, DISPLAY_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, INDICATORS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, MODE_SELECTOR_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, DATA_SELECTOR_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, WAYPOINT_SELECTOR_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, AUTO_MAN_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, CROSS_TRACK_ERROR_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, DESIRED_TRACK_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, EVENT, "NUMBER");

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA,
                                    SIMCONNECT_OBJECT_ID_USER_AIRCRAFT, SIMCONNECT_PERIOD_VISUAL_FRAME);
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
        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
          SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA *)pData;
          DATA *data = (DATA *)&pObjData->dwData;
          varManager->setVar(SIM_VAR_AIRSPEED_TRUE, data->airspeedTrue);
          varManager->setVar(SIM_VAR_AMBIENT_TEMPERATURE, data->ambientTemp);
          varManager->setVar(SIM_VAR_AMBIENT_WIND_DIRECTION, data->windDirection);
          varManager->setVar(SIM_VAR_AMBIENT_WIND_VELOCITY, data->windSpeed);
          varManager->setVar(SIM_VAR_GROUND_VELOCITY, data->groundSpeed);
          varManager->setVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, data->headingTrue);
          varManager->setVar(SIM_VAR_PLANE_LATITUDE, data->latitude);
          varManager->setVar(SIM_VAR_PLANE_LONGITUDE, data->longitude);
          varManager->setVar(SIM_VAR_NAV_DME_1, data->navDME1);
          varManager->setVar(SIM_VAR_NAV_DME_2, data->navDME2);
          varManager->setVar(SIM_VAR_SIMULATION_RATE, data->simRate);

          handleEvent(data->event);
          data->event = 0;

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

static void exportVars() {
  EXPORT export1 = { 0 };
  EXPORT export2 = { 0 };
  EXPORT export3 = { 0 };

  varManager->getVar(DISPLAY_VAR_UNIT_1, export1.displays);
  varManager->getVar(INDICATORS_VAR_UNIT_1, export1.indicators);
  varManager->getVar(MODE_SELECTOR_POS_VAR_UNIT_1, export1.modeSelectorPos);
  varManager->getVar(DATA_SELECTOR_POS_VAR_UNIT_1, export1.dataSelectorPos);
  varManager->getVar(WAYPOINT_SELECTOR_POS_VAR_UNIT_1, export1.waypointSelectorPos);
  varManager->getVar(AUTO_MAN_POS_VAR_UNIT_1, export1.autoManPos);
  varManager->getVar(CROSS_TRACK_ERROR_VAR_UNIT_1, export1.crossTrackError);
  varManager->getVar(DESIRED_TRACK_VAR_UNIT_1, export1.desiredTrack);

  varManager->getVar(DISPLAY_VAR_UNIT_2, export2.displays);
  varManager->getVar(INDICATORS_VAR_UNIT_2, export2.indicators);
  varManager->getVar(MODE_SELECTOR_POS_VAR_UNIT_2, export2.modeSelectorPos);
  varManager->getVar(DATA_SELECTOR_POS_VAR_UNIT_2, export2.dataSelectorPos);
  varManager->getVar(WAYPOINT_SELECTOR_POS_VAR_UNIT_2, export2.waypointSelectorPos);
  varManager->getVar(AUTO_MAN_POS_VAR_UNIT_2, export2.autoManPos);
  varManager->getVar(CROSS_TRACK_ERROR_VAR_UNIT_2, export2.crossTrackError);
  varManager->getVar(DESIRED_TRACK_VAR_UNIT_2, export2.desiredTrack);

  varManager->getVar(DISPLAY_VAR_UNIT_3, export3.displays);
  varManager->getVar(INDICATORS_VAR_UNIT_3, export3.indicators);
  varManager->getVar(MODE_SELECTOR_POS_VAR_UNIT_3, export3.modeSelectorPos);
  varManager->getVar(DATA_SELECTOR_POS_VAR_UNIT_3, export3.dataSelectorPos);
  varManager->getVar(WAYPOINT_SELECTOR_POS_VAR_UNIT_3, export3.waypointSelectorPos);
  varManager->getVar(AUTO_MAN_POS_VAR_UNIT_3, export3.autoManPos);
  varManager->getVar(CROSS_TRACK_ERROR_VAR_UNIT_3, export3.crossTrackError);
  varManager->getVar(DESIRED_TRACK_VAR_UNIT_3, export3.desiredTrack);

  SimConnect_SetDataOnSimObject(simConnect, DATA_DEFINITIONS_UNIT_1, SIMCONNECT_OBJECT_ID_USER_AIRCRAFT,
                                SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(EXPORT), &export1);
  SimConnect_SetDataOnSimObject(simConnect, DATA_DEFINITIONS_UNIT_2, SIMCONNECT_OBJECT_ID_USER_AIRCRAFT,
                                SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(EXPORT), &export2);
  SimConnect_SetDataOnSimObject(simConnect, DATA_DEFINITIONS_UNIT_3, SIMCONNECT_OBJECT_ID_USER_AIRCRAFT,
                                SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(EXPORT), &export3);
}

#pragma endregion

extern "C" MSFS_CALLBACK void module_init(void) {
  varManager = std::make_unique<VarManager>();

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
  ins->update(dTime * simRate);

  exportVars();

  return true;
}

extern "C" MSFS_CALLBACK void module_deinit(void) {
  // This is called when the module is deinitialized

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA,
                                    SIMCONNECT_OBJECT_ID_USER_AIRCRAFT, SIMCONNECT_PERIOD_NEVER);
}
