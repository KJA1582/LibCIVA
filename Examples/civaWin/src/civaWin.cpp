#include "civaWin.h"

std::unique_ptr<WinVarManager> winVarManager;
std::unique_ptr<libciva::INSContainer> ins;

// Pure AP Demo
std::unique_ptr<LateralAutopilot> lateralAutopilot;
std::unique_ptr<VerticalAutopilot> verticalAutopilot;

std::thread INSThread;
std::mutex lock;
std::atomic<bool> __exit;
std::atomic<uint8_t> selectedUnit = 4;
std::atomic<bool> power = false;
std::atomic<bool> simPaused = false;

HANDLE simConnect = 0;
SIMCONNECT_RECV_OPEN openData;

static void handleSimConnect() {
  SIMCONNECT_RECV *pData;
  DWORD cbData;

  HRESULT hr = SimConnect_GetNextDispatch(simConnect, &pData, &cbData);
  if (SUCCEEDED(hr)) {
    switch (pData->dwID) {
      case SIMCONNECT_RECV_ID_EVENT: {
        SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT *)pData;
        if (evt->uEventID == EVENT_ID_PAUSE) {
          simPaused = evt->dwData != 0;
        }
        break;
      }
      case SIMCONNECT_RECV_ID_OPEN: {
        openData = *(SIMCONNECT_RECV_OPEN *)pData;
        break;
      }
      case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA *)pData;
        DATA *data = (DATA *)&pObjData->dwData;
        winVarManager->sim.airspeedTrue = data->airspeedTrue;
        winVarManager->sim.groundVelocity = data->groundVelocity;
        winVarManager->sim.ambientTemperature = data->ambientTemp;
        winVarManager->sim.ambientWindDirection = data->windDirection;
        winVarManager->sim.ambientWindVelocity = data->windSpeed;
        winVarManager->sim.planeHeadingDegreesTrue = data->headingTrue;
        winVarManager->sim.planeLatitude = data->latitude;
        winVarManager->sim.planeLongitude = data->longitude;
        winVarManager->sim.navDme1 = data->navDME1;
        winVarManager->sim.navDme2 = data->navDME2;
        winVarManager->sim.simulationRate = data->simRate;
        winVarManager->sim.planeAltitude = data->altitude;

        // Pure AP Demo
        winVarManager->rollRate = data->rollRateBodyZ;
        winVarManager->bankAngle = data->planeBankDegrees;
        winVarManager->pitchRate = data->pitchRateBodyX;
        winVarManager->pitchAngle = data->planePitchDegrees;

        break;
      }
      case SIMCONNECT_RECV_ID_EXCEPTION: {
        SIMCONNECT_RECV_EXCEPTION *except = (SIMCONNECT_RECV_EXCEPTION *)pData;
        libciva::Logger::GetInstance() << libciva::Logger::GetInstance().time() << "SimConnect Exception: " << except->dwException
                                       << "\n";
        break;
      }
      default:
        break;
    }
  }
}

static void runner() {
  std::chrono::steady_clock::time_point prev = std::chrono::steady_clock::now();

  while (!__exit) {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::nanoseconds delta = now - prev;
    prev = now;

    // If SC, call and process data;
    if (simConnect != NULL) {
      handleSimConnect();
    }

    {
      std::lock_guard<std::mutex> guard(lock);

      if (!simPaused) {
        ins->update(delta.count() * 1e-9 * winVarManager->sim.simulationRate);

        // Pure AP Demo
        if (lateralAutopilot->isEnabled() && winVarManager->unit[0].valid != (double)libciva::SIGNAL_VALIDITY::NAV)
          lateralAutopilot->disable();

        lateralAutopilot->update(delta.count() * 1e-9 * winVarManager->sim.simulationRate, winVarManager->bankAngle,
                                 winVarManager->rollRate, winVarManager->unit[0].track, winVarManager->unit[0].crossTrackError,
                                 winVarManager->unit[0].trackAngleError, winVarManager->unit[0].desiredTrack);

        verticalAutopilot->update(delta.count() * 1e-9 * winVarManager->sim.simulationRate, winVarManager->sim.planeAltitude,
                                  winVarManager->pitchAngle, winVarManager->pitchRate);

        if (simConnect != NULL && lateralAutopilot->isEnabled()) {
          int16_t aileron = lateralAutopilot->getOutput();
          SimConnect_TransmitClientEvent(simConnect, SIMCONNECT_OBJECT_ID_USER, EVENT_DEFINITIONS_AILERON_SET, aileron,
                                         SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }

        if (simConnect != NULL && verticalAutopilot->isEnabled()) {
          int16_t elevator = verticalAutopilot->getOutput();
          SimConnect_TransmitClientEvent(simConnect, SIMCONNECT_OBJECT_ID_USER, EVENT_DEFINITIONS_ELEVATOR_SET, elevator,
                                         SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }
      }
    }

    HANDLE handle;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordinates = {};
    SetConsoleCursorPosition(handle, coordinates);

    winVarManager->dump();
    std::cout << std::endl;
    if (simConnect == NULL) {
      std::cout << "No SimConnect" << std::endl;
    } else {
      std::cout << "SimConnect " << openData.szApplicationName << " Version " << openData.dwApplicationVersionMajor << "."
                << openData.dwApplicationVersionMinor << "." << openData.dwApplicationBuildMajor << "."
                << openData.dwApplicationBuildMinor << std::endl
                << std::endl;
    }

    static constexpr int COL_WIDTH = 29;

    if (selectedUnit == 4)
      std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "All units selected" << std::endl << std::endl;
    else
      std::cout << "Selected Unit: " << std::left << std::setfill(' ') << std::setw(14) << (double)selectedUnit << std::endl
                << std::endl;

    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "Unit Sel  : 1,2,3,4 (All)";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "Data Entry: Numpad";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "Mode Knob : Arrow left/right" << std::endl;
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "Data Knob : Arrow up/down";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "WPT Sel   : Numpad +/-";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "INSERT    : Numpad enter" << std::endl;
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "TEST      : T";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "DME LL    : L";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "DME FREQ  : F" << std::endl;
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "CLEAR     : DEL";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "WPT CHG   : W";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "HOLD      : H" << std::endl;
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "AUTO/MAN  : A";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "REMOTE    : R";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "INST ALIGN: I" << std::endl;
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "AC POWER  : P";
    std::cout << std::left << std::setfill(' ') << std::setw(COL_WIDTH) << "SB Import : U" << std::endl;

    // Pure AP Demo
    std::cout << std::endl;
    std::cout << "AP LNAV   : G (" << std::left << std::setfill(' ') << std::setw(14)
              << (lateralAutopilot->isEnabled() ? "ENABLED)" : "DISABLED)");
    std::cout << "AP ALT HLD: V (" << std::left << std::setfill(' ') << std::setw(14)
              << (verticalAutopilot->isEnabled() ? "ENABLED)" : "DISABLED)") << std::endl;

    std::cout << std::endl << "dT was " << delta.count() * 1e-6 << "ms" << std::endl;
  }
}

static void setupSimConnect() {
  HRESULT hr;

  hr = SimConnect_Open(&simConnect, "civaWin", NULL, 0, NULL, 0);
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
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, libciva::SIM_VAR_PLANE_ALTITUDE, "FEET");

  // Pure AP Demo
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, "ROTATION VELOCITY BODY Z", "DEGREE PER SECOND");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, "PLANE BANK DEGREES", "DEGREE");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, "ROTATION VELOCITY BODY X", "DEGREE PER SECOND");
  SimConnect_AddToDataDefinition(simConnect, DATA_DEFINITIONS_DATA, "PLANE PITCH DEGREES", "DEGREE");
  SimConnect_MapClientEventToSimEvent(simConnect, EVENT_DEFINITIONS_AILERON_SET, "AILERON_SET");
  SimConnect_MapClientEventToSimEvent(simConnect, EVENT_DEFINITIONS_ELEVATOR_SET, "ELEVATOR_SET");

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA, SIMCONNECT_OBJECT_ID_USER,
                                    SIMCONNECT_PERIOD_VISUAL_FRAME, SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
}

static void importSimBrief(std::shared_ptr<libciva::INS> unit1, std::shared_ptr<libciva::INS> unit2,
                           std::shared_ptr<libciva::INS> unit3) {
  std::ifstream ifs{"../../simBrief.json"};

  rapidjson::IStreamWrapper isw{ifs};

  rapidjson::Document doc{};
  doc.ParseStream(isw);

  uint8_t index = 0;
  libciva::POSITION wpt[9] = {999};

  rapidjson::GenericArray<false, rapidjson::Value> fixes = doc["navlog"]["fix"].GetArray();
  for (rapidjson::GenericValue<rapidjson::UTF8<>> &i : fixes) {
    if (index >= 9) continue;

    std::string ident = std::string(i["ident"].GetString());

    if (ident != "TOD" && ident != "TOC") {
      wpt[index].latitude = atof(i["pos_lat"].GetString());
      wpt[index].longitude = atof(i["pos_long"].GetString());

      index++;
    }
  }

  if (unit1) unit1->remoteInsertWPT(wpt);
  if (unit2) unit2->remoteInsertWPT(wpt);
  if (unit3) unit3->remoteInsertWPT(wpt);
}

int main() {
  libciva::Logger::GetInstance() << libciva::Logger::GetInstance().time() << "civaWIN Example booting"
                                 << "\n";

  setupSimConnect();

  winVarManager = std::make_unique<WinVarManager>();

  ins = std::make_unique<libciva::INSContainer>(*winVarManager, libciva::UNIT_COUNT::THREE, libciva::UNIT_HAS_DME::BOTH, "", true,
                                                false);

  // Pure AP Demo
  lateralAutopilot = std::make_unique<LateralAutopilot>();
  verticalAutopilot = std::make_unique<VerticalAutopilot>();

  INSThread = std::thread(runner);

  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  INPUT_RECORD inp;
  DWORD num_of_events;

  // Mainloop
  while (!__exit) {
    ReadConsoleInput(hIn, &inp, 1, &num_of_events);

    switch (inp.Event.KeyEvent.wVirtualKeyCode) {
      case '1':
      case '2':
      case '3':
      case '4':
        selectedUnit = (const uint8_t)(inp.Event.KeyEvent.wVirtualKeyCode - '0');
        continue;
    }

    ins->handleEvent([inp](auto unit1, auto unit2, auto unit3) {
      std::lock_guard<std::mutex> guard(lock);

      switch (inp.EventType) {
        case KEY_EVENT: {
          if (!inp.Event.KeyEvent.bKeyDown) {
            if (inp.Event.KeyEvent.wVirtualKeyCode == 'T') {
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleTestButtonState(false);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleTestButtonState(false);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleTestButtonState(false);
            }

            break;
          }

          switch (inp.Event.KeyEvent.wVirtualKeyCode) {
            case VK_UP:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->incDataSelectorPos();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->incDataSelectorPos();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->incDataSelectorPos();
              break;
            case VK_DOWN:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->decDataSelectorPos();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->decDataSelectorPos();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->decDataSelectorPos();
              break;
            case VK_LEFT:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->decModeSelectorPos();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->decModeSelectorPos();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->decModeSelectorPos();
              break;
            case VK_RIGHT:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->incModeSelectorPos();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->incModeSelectorPos();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->incModeSelectorPos();
              break;
            case VK_NUMPAD0:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(0);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(0);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(0);
              break;
            case VK_NUMPAD1:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(1);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(1);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(1);
              break;
            case VK_NUMPAD2:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(2);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(2);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(2);
              break;
            case VK_NUMPAD3:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(3);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(3);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(3);
              break;
            case VK_NUMPAD4:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(4);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(4);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(4);
              break;
            case VK_NUMPAD5:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(5);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(5);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(5);
              break;
            case VK_NUMPAD6:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(6);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(6);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(6);
              break;
            case VK_NUMPAD7:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(7);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(7);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(7);
              break;
            case VK_NUMPAD8:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(8);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(8);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(8);
              break;
            case VK_NUMPAD9:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleNumeric(9);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleNumeric(9);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleNumeric(9);
              break;
            case VK_RETURN:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleInsert();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleInsert();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleInsert();
              break;
            case VK_ADD:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->incWaypointSelectorPos();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->incWaypointSelectorPos();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->incWaypointSelectorPos();
              break;
            case VK_SUBTRACT:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->decWaypointSelectorPos();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->decWaypointSelectorPos();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->decWaypointSelectorPos();
              break;
            case VK_DELETE:
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleClear();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleClear();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleClear();
              break;
            case 'T':
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleTestButtonState(true);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleTestButtonState(true);
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleTestButtonState(true);
              Sleep(200); // DEBOUNCE;
              break;
            case 'L':
            case 'F':
              if (selectedUnit == 1 || selectedUnit == 4)
                unit1->handleDMEModeEntry((const uint8_t)inp.Event.KeyEvent.wVirtualKeyCode);
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4))
                unit2->handleDMEModeEntry((const uint8_t)inp.Event.KeyEvent.wVirtualKeyCode);
              if (unit2 && (selectedUnit == 3 || selectedUnit == 4))
                unit3->handleDMEModeEntry((const uint8_t)inp.Event.KeyEvent.wVirtualKeyCode);
              break;
            case 'W':
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleWaypointChange();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleWaypointChange();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleWaypointChange();
              break;
            case 'H':
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleHoldButton();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleHoldButton();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleHoldButton();
              break;
            case 'A':
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleAutoMan();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleAutoMan();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleAutoMan();
              break;
            case 'R':
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleRemote();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleRemote();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleRemote();
              break;
            case 'I':
              if (selectedUnit == 1 || selectedUnit == 4) unit1->handleInstantAlign();
              if (unit2 && (selectedUnit == 2 || selectedUnit == 4)) unit2->handleInstantAlign();
              if (unit3 && (selectedUnit == 3 || selectedUnit == 4)) unit3->handleInstantAlign();
              break;
            case 'P':
              power = !power;
              unit1->handleExternalPower(power);
              if (unit2) unit2->handleExternalPower(power);
              if (unit3) unit3->handleExternalPower(power);
              break;
            case 'U':
              importSimBrief(unit1, unit2, unit3);
              break;

              // Pure AP Demo
            case 'G':
              if (lateralAutopilot->isEnabled()) {
                lateralAutopilot->disable();
              } else {
                lateralAutopilot->enable();
              }
              break;
            case 'V':
              if (verticalAutopilot->isEnabled()) {
                verticalAutopilot->disable();
              } else {
                verticalAutopilot->enable(winVarManager->sim.planeAltitude);
              }
              break;

            case VK_ESCAPE:
              __exit = true;
              break;
          }

          break;
        }
      }
    });

    Sleep(20);
  }

  INSThread.join();

  return 0;
}
