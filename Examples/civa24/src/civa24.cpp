#include "civa24.h"

std::unique_ptr<libciva::VarManager> varManager;
std::unique_ptr<libciva::INSContainer> ins;

HANDLE simConnect = 0;
bool simPaused = false;

#pragma region Preset LVAR names

constexpr auto EVENT = "L:LIBCIVA_EVENT";

std::string DISPLAY_VAR_UNIT_1 = std::string("L") + libciva::DISPLAY_VAR + libciva::ID_UNIT_1;
std::string INDICATORS_VAR_UNIT_1 = std::string("L") + libciva::INDICATORS_VAR + libciva::ID_UNIT_1;
std::string MODE_SELECTOR_POS_VAR_UNIT_1 = std::string("L") + libciva::MODE_SELECTOR_POS_VAR + libciva::ID_UNIT_1;
std::string DATA_SELECTOR_POS_VAR_UNIT_1 = std::string("L") + libciva::DATA_SELECTOR_POS_VAR + libciva::ID_UNIT_1;
std::string WAYPOINT_SELECTOR_POS_VAR_UNIT_1 = std::string("L") + libciva::WAYPOINT_SELECTOR_POS_VAR + libciva::ID_UNIT_1;
std::string AUTO_MAN_POS_VAR_UNIT_1 = std::string("L") + libciva::AUTO_MAN_POS_VAR + libciva::ID_UNIT_1;
std::string CROSS_TRACK_ERROR_VAR_UNIT_1 = std::string("L") + libciva::CROSS_TRACK_ERROR_VAR + libciva::ID_UNIT_1;
std::string DESIRED_TRACK_VAR_UNIT_1 = std::string("L") + libciva::DESIRED_TRACK_VAR + libciva::ID_UNIT_1;
std::string TRACK_VAR_UNIT_1 = std::string("L") + libciva::TRACK_VAR + libciva::ID_UNIT_1;
std::string TRACK_ANGLE_ERROR_VAR_UNIT_1 = std::string("L") + libciva::TRACK_ANGLE_ERROR_VAR + libciva::ID_UNIT_1;
std::string DISTANCE_VAR_UNIT_1 = std::string("L") + libciva::DISTANCE_VAR + libciva::ID_UNIT_1;
std::string GROUND_SPEED_VAR_UNIT_1 = std::string("L") + libciva::GROUND_SPEED_VAR + libciva::ID_UNIT_1;
std::string VALID_UNIT_1 = std::string("L") + libciva::VALID + libciva::ID_UNIT_1;

std::string DISPLAY_VAR_UNIT_2 = std::string("L") + libciva::DISPLAY_VAR + libciva::ID_UNIT_2;
std::string INDICATORS_VAR_UNIT_2 = std::string("L") + libciva::INDICATORS_VAR + libciva::ID_UNIT_2;
std::string MODE_SELECTOR_POS_VAR_UNIT_2 = std::string("L") + libciva::MODE_SELECTOR_POS_VAR + libciva::ID_UNIT_2;
std::string DATA_SELECTOR_POS_VAR_UNIT_2 = std::string("L") + libciva::DATA_SELECTOR_POS_VAR + libciva::ID_UNIT_2;
std::string WAYPOINT_SELECTOR_POS_VAR_UNIT_2 = std::string("L") + libciva::WAYPOINT_SELECTOR_POS_VAR + libciva::ID_UNIT_2;
std::string AUTO_MAN_POS_VAR_UNIT_2 = std::string("L") + libciva::AUTO_MAN_POS_VAR + libciva::ID_UNIT_2;
std::string CROSS_TRACK_ERROR_VAR_UNIT_2 = std::string("L") + libciva::CROSS_TRACK_ERROR_VAR + libciva::ID_UNIT_2;
std::string DESIRED_TRACK_VAR_UNIT_2 = std::string("L") + libciva::DESIRED_TRACK_VAR + libciva::ID_UNIT_2;
std::string TRACK_VAR_UNIT_2 = std::string("L") + libciva::TRACK_VAR + libciva::ID_UNIT_2;
std::string TRACK_ANGLE_ERROR_VAR_UNIT_2 = std::string("L") + libciva::TRACK_ANGLE_ERROR_VAR + libciva::ID_UNIT_2;
std::string DISTANCE_VAR_UNIT_2 = std::string("L") + libciva::DISTANCE_VAR + libciva::ID_UNIT_2;
std::string GROUND_SPEED_VAR_UNIT_2 = std::string("L") + libciva::GROUND_SPEED_VAR + libciva::ID_UNIT_2;
std::string VALID_UNIT_2 = std::string("L") + libciva::VALID + libciva::ID_UNIT_2;

std::string DISPLAY_VAR_UNIT_3 = std::string("L") + libciva::DISPLAY_VAR + libciva::ID_UNIT_3;
std::string INDICATORS_VAR_UNIT_3 = std::string("L") + libciva::INDICATORS_VAR + libciva::ID_UNIT_3;
std::string MODE_SELECTOR_POS_VAR_UNIT_3 = std::string("L") + libciva::MODE_SELECTOR_POS_VAR + libciva::ID_UNIT_3;
std::string DATA_SELECTOR_POS_VAR_UNIT_3 = std::string("L") + libciva::DATA_SELECTOR_POS_VAR + libciva::ID_UNIT_3;
std::string WAYPOINT_SELECTOR_POS_VAR_UNIT_3 = std::string("L") + libciva::WAYPOINT_SELECTOR_POS_VAR + libciva::ID_UNIT_3;
std::string AUTO_MAN_POS_VAR_UNIT_3 = std::string("L") + libciva::AUTO_MAN_POS_VAR + libciva::ID_UNIT_3;
std::string CROSS_TRACK_ERROR_VAR_UNIT_3 = std::string("L") + libciva::CROSS_TRACK_ERROR_VAR + libciva::ID_UNIT_3;
std::string DESIRED_TRACK_VAR_UNIT_3 = std::string("L") + libciva::DESIRED_TRACK_VAR + libciva::ID_UNIT_3;
std::string TRACK_VAR_UNIT_3 = std::string("L") + libciva::TRACK_VAR + libciva::ID_UNIT_3;
std::string TRACK_ANGLE_ERROR_VAR_UNIT_3 = std::string("L") + libciva::TRACK_ANGLE_ERROR_VAR + libciva::ID_UNIT_3;
std::string DISTANCE_VAR_UNIT_3 = std::string("L") + libciva::DISTANCE_VAR + libciva::ID_UNIT_3;
std::string GROUND_SPEED_VAR_UNIT_3 = std::string("L") + libciva::GROUND_SPEED_VAR + libciva::ID_UNIT_3;
std::string VALID_UNIT_3 = std::string("L") + libciva::VALID + libciva::ID_UNIT_3;

#pragma endregion

#pragma region Events

static void handleEvent(int event) {
  bool isUnit1 = event < 100;
  bool isUnit2 = event > 100 && event < 200;
  bool isUnit3 = event > 200 && event < 300;
  event = event % 100;

  ins->handleEvent([event, isUnit1, isUnit2, isUnit3](auto unit1, auto unit2, auto unit3) {
    switch (event) {
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
        if (isUnit1) unit1->handleNumeric(event - 1);
        if (isUnit2 && unit2) unit2->handleNumeric(event - 1);
        if (isUnit3 && unit3) unit3->handleNumeric(event - 1);
        break;
      case EVENT_INC_MODE:
        if (isUnit1) unit1->incModeSelectorPos();
        if (isUnit2 && unit2) unit2->incModeSelectorPos();
        if (isUnit3 && unit3) unit3->incModeSelectorPos();
        break;
      case EVENT_DEC_MODE:
        if (isUnit1) unit1->decModeSelectorPos();
        if (isUnit2 && unit2) unit2->decModeSelectorPos();
        if (isUnit3 && unit3) unit3->decModeSelectorPos();
        break;
      case EVENT_INC_DATA:
        if (isUnit1) unit1->incDataSelectorPos();
        if (isUnit2 && unit2) unit2->incDataSelectorPos();
        if (isUnit3 && unit3) unit3->incDataSelectorPos();
        break;
      case EVENT_DEC_DATA:
        if (isUnit1) unit1->decDataSelectorPos();
        if (isUnit2 && unit2) unit2->decDataSelectorPos();
        if (isUnit3 && unit3) unit3->decDataSelectorPos();
        break;
      case EVENT_INC_WPT:
        if (isUnit1) unit1->incWaypointSelectorPos();
        if (isUnit2 && unit2) unit2->incWaypointSelectorPos();
        if (isUnit3 && unit3) unit3->incWaypointSelectorPos();
        break;
      case EVENT_DEC_WPT:
        if (isUnit1) unit1->decWaypointSelectorPos();
        if (isUnit2 && unit2) unit2->decWaypointSelectorPos();
        if (isUnit3 && unit3) unit3->decWaypointSelectorPos();
        break;
      case EVENT_INSERT:
        if (isUnit1) unit1->handleInsert();
        if (isUnit2 && unit2) unit2->handleInsert();
        if (isUnit3 && unit3) unit3->handleInsert();
        break;
      case EVENT_TEST_DOWN:
        if (isUnit1) unit1->handleTestButtonState(true);
        if (isUnit2 && unit2) unit2->handleTestButtonState(true);
        if (isUnit3 && unit3) unit3->handleTestButtonState(true);
        break;
      case EVENT_TEST_UP:
        if (isUnit1) unit1->handleTestButtonState(false);
        if (isUnit2 && unit2) unit2->handleTestButtonState(false);
        if (isUnit3 && unit3) unit3->handleTestButtonState(false);
        break;
      case EVENT_DME_LL:
        if (isUnit1) unit1->handleDMEModeEntry('L');
        if (isUnit2 && unit2) unit2->handleDMEModeEntry('L');
        if (isUnit3 && unit3) unit3->handleDMEModeEntry('L');
        break;
      case EVENT_DME_FREQ:
        if (isUnit1) unit1->handleDMEModeEntry('F');
        if (isUnit2 && unit2) unit2->handleDMEModeEntry('F');
        if (isUnit3 && unit3) unit3->handleDMEModeEntry('F');
        break;
      case EVENT_CLEAR:
        if (isUnit1) unit1->handleClear();
        if (isUnit2 && unit2) unit2->handleClear();
        if (isUnit3 && unit3) unit3->handleClear();
        break;
      case EVENT_WPT_CHG:
        if (isUnit1) unit1->handleWaypointChange();
        if (isUnit2 && unit2) unit2->handleWaypointChange();
        if (isUnit3 && unit3) unit3->handleWaypointChange();
        break;
      case EVENT_HOLD:
        if (isUnit1) unit1->handleHoldButton();
        if (isUnit2 && unit2) unit2->handleHoldButton();
        if (isUnit3 && unit3) unit3->handleHoldButton();
        break;
      case EVENT_AUTO_MAN:
        if (isUnit1) unit1->handleAutoMan();
        if (isUnit2 && unit2) unit2->handleAutoMan();
        if (isUnit3 && unit3) unit3->handleAutoMan();
        break;
      case EVENT_REMOTE:
        if (isUnit1) unit1->handleRemote();
        if (isUnit2 && unit2) unit2->handleRemote();
        if (isUnit3 && unit3) unit3->handleRemote();
        break;
      case EVENT_INSTANT_ALIGN:
        if (isUnit1) unit1->handleInstantAlign();
        if (isUnit2 && unit2) unit2->handleInstantAlign();
        if (isUnit3 && unit3) unit3->handleInstantAlign();
        break;
      case EVENT_EXTERNAL_POWER_ON:
        unit1->handleExternalPower(true);
        if (unit2) unit2->handleExternalPower(true);
        if (unit3) unit3->handleExternalPower(true);
        break;
      case EVENT_EXTERNAL_POWER_OFF:
        unit1->handleExternalPower(false);
        if (unit2) unit2->handleExternalPower(false);
        if (unit3) unit3->handleExternalPower(false);
        break;
      default:
        libciva::Logger::GetInstance() << "Unknown event " << event << "\n";
    }
  });

  if (event != 0) {
    double reset = 0;
    SimConnect_SetDataOnSimObject(simConnect, DATA_DEFINITIONS_EVENT, SIMCONNECT_SIMOBJECT_TYPE_USER_AIRCRAFT,
                                  SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(double), &reset);
  }
}

#pragma endregion

#pragma region SimConnect

static void setupSimConnect() {
  HRESULT hr;

  hr = SimConnect_Open(&simConnect, "libciva", NULL, 0, NULL, 0);
  if (FAILED(hr)) return;

  SimConnect_SubscribeToSystemEvent(simConnect, EVENT_ID_PAUSE, "Pause_EX1");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_AIRSPEED_TRUE, "KNOT");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_GROUND_VELOCITY, "KNOT");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_AMBIENT_TEMPERATURE, "CELSIUS");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_AMBIENT_WIND_DIRECTION, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_AMBIENT_WIND_VELOCITY, "KNOT");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_PLANE_HEADING_DEGREES_TRUE, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_PLANE_LATITUDE, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_PLANE_LONGITUDE, "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_NAV_DME_1, "NAUTICAL MILE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_NAV_DME_2, "NAUTICAL MILE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_SIMULATION_RATE, "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, EVENT, "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, "ATC ID", NULL, SIMCONNECT_DATATYPE_STRING32);
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, "PLANE ALTITUDE", "FEET");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_EVENT, EVENT, "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, DISPLAY_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, INDICATORS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, MODE_SELECTOR_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, DATA_SELECTOR_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, WAYPOINT_SELECTOR_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, AUTO_MAN_POS_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, CROSS_TRACK_ERROR_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, DESIRED_TRACK_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, TRACK_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, TRACK_ANGLE_ERROR_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, DISTANCE_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, GROUND_SPEED_VAR_UNIT_1.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_1, VALID_UNIT_1.c_str(), "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, DISPLAY_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, INDICATORS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, MODE_SELECTOR_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, DATA_SELECTOR_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, WAYPOINT_SELECTOR_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, AUTO_MAN_POS_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, CROSS_TRACK_ERROR_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, DESIRED_TRACK_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, TRACK_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, TRACK_ANGLE_ERROR_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, DISTANCE_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, GROUND_SPEED_VAR_UNIT_2.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_2, VALID_UNIT_2.c_str(), "NUMBER");

  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, DISPLAY_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, INDICATORS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, MODE_SELECTOR_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, DATA_SELECTOR_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, WAYPOINT_SELECTOR_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, AUTO_MAN_POS_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, CROSS_TRACK_ERROR_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, DESIRED_TRACK_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, TRACK_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, TRACK_ANGLE_ERROR_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, DISTANCE_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, GROUND_SPEED_VAR_UNIT_3.c_str(), "NUMBER");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_UNIT_3, VALID_UNIT_3.c_str(), "NUMBER");

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA,
                                    SIMCONNECT_OBJECT_ID_USER_AIRCRAFT, SIMCONNECT_PERIOD_VISUAL_FRAME,
                                    SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
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

        case SIMCONNECT_RECV_ID_EVENT: {
          SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT *)pData;
          if (evt->uEventID == EVENT_ID_PAUSE) {
            simPaused = evt->dwData != 0;
          }
          break;
        }

        case SIMCONNECT_RECV_ID_OPEN: {
          SIMCONNECT_RECV_OPEN *openData = (SIMCONNECT_RECV_OPEN *)pData;
          libciva::Logger::GetInstance() << "Connected to " << openData->szApplicationName << "\n";
          break;
        }
        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
          SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA *)pData;
          DATA *data = (DATA *)&pObjData->dwData;
          varManager->sim.airspeedTrue = data->airspeedTrue;
          varManager->sim.groundVelocity = data->groundVelocity;
          varManager->sim.ambientTemperature = data->ambientTemp;
          varManager->sim.ambientWindDirection = data->windDirection;
          varManager->sim.ambientWindVelocity = data->windSpeed;
          varManager->sim.planeHeadingDegreesTrue = data->headingTrue;
          varManager->sim.planeLatitude = data->latitude;
          varManager->sim.planeLongitude = data->longitude;
          varManager->sim.navDme1 = data->navDME1;
          varManager->sim.navDme2 = data->navDME2;
          varManager->sim.simulationRate = data->simRate;
          varManager->sim.planeAltitude = data->altitude;

          if (data->atcID != NULL && !ins) {
            libciva::Logger::GetInstance() << "Booting INS for " << data->atcID;
            ins = std::make_unique<libciva::INSContainer>(*varManager, libciva::UNIT_COUNT::THREE, libciva::UNIT_HAS_DME::BOTH,
                                                          data->atcID, true, true);
          }

          if (data->event != 0) {
            handleEvent((int)data->event);
            data->event = 0;
          }

          break;
        }
        case SIMCONNECT_RECV_ID_EXCEPTION: {
          SIMCONNECT_RECV_EXCEPTION *except = (SIMCONNECT_RECV_EXCEPTION *)pData;
          libciva::Logger::GetInstance() << "SimConnect Exception: " << except->dwException << "\n";
          break;
        }
        default:
          break;
      }
    } else {
      break;
    }
  }
}

static void exportVars() {
  SimConnect_SetDataOnSimObject(simConnect, DATA_DEFINITIONS_UNIT_1, SIMCONNECT_OBJECT_ID_USER_AIRCRAFT,
                                SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(libciva::VarManager::UnitExport),
                                &varManager->unit[0]);
  SimConnect_SetDataOnSimObject(simConnect, DATA_DEFINITIONS_UNIT_2, SIMCONNECT_OBJECT_ID_USER_AIRCRAFT,
                                SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(libciva::VarManager::UnitExport),
                                &varManager->unit[1]);
  SimConnect_SetDataOnSimObject(simConnect, DATA_DEFINITIONS_UNIT_3, SIMCONNECT_OBJECT_ID_USER_AIRCRAFT,
                                SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(libciva::VarManager::UnitExport),
                                &varManager->unit[2]);
}

#pragma endregion

extern "C" MSFS_CALLBACK bool LibCIVA_gauge_init(FsContext ctx, sGaugeInstallData *p_install_data) {
  libciva::Logger::GetInstance() << "Starting libCIVA\n";

  setupSimConnect();

  varManager = std::make_unique<libciva::VarManager>();

  return true;
}

extern "C" MSFS_CALLBACK bool LibCIVA_gauge_update(FsContext ctx, float dTime) {
  // This is called each frame
  // dTime is the time between the previous frame and this one
  // return false if there is any error

  handleSimConnect();

  if (!simPaused) {
    ins->update(dTime * varManager->sim.simulationRate);
  }

  exportVars();

  return true;
}

extern "C" MSFS_CALLBACK bool LibCIVA_gauge_kill(FsContext ctx) {
  ins->~INSContainer();

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA,
                                    SIMCONNECT_OBJECT_ID_USER_AIRCRAFT, SIMCONNECT_PERIOD_NEVER);

  libciva::Logger::GetInstance() << "Stopped libCIVA\n";
  libciva::Logger::GetInstance().~Logger();

  return true;
}
