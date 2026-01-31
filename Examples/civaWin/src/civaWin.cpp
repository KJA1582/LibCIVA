#include "civaWin.h"

std::unique_ptr<WinVarManager> winVarManager;
std::unique_ptr<INSContainer> ins;

std::thread INSThread;
std::mutex lock;
std::atomic<bool> __exit;

HANDLE simConnect = 0;
SIMCONNECT_RECV_OPEN openData;

static void handleSimConnect() {
  SIMCONNECT_RECV *pData;
  DWORD cbData;

  HRESULT hr = SimConnect_GetNextDispatch(simConnect, &pData, &cbData);
  if (SUCCEEDED(hr)) {
    switch (pData->dwID) {
      case SIMCONNECT_RECV_ID_OPEN: {
        openData = *(SIMCONNECT_RECV_OPEN *)pData;
        break;
      }
      case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA *)pData;
        DATA *data = (DATA *)&pObjData->dwData;
        winVarManager->setVar(SIM_VAR_AIRSPEED_TRUE, data->airspeedTrue);
        winVarManager->setVar(SIM_VAR_AMBIENT_TEMPERATURE, data->ambientTemp);
        winVarManager->setVar(SIM_VAR_AMBIENT_WIND_DIRECTION, data->windDirection);
        winVarManager->setVar(SIM_VAR_AMBIENT_WIND_VELOCITY, data->windSpeed);
        winVarManager->setVar(SIM_VAR_GROUND_VELOCITY, data->groundSpeed);
        winVarManager->setVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, data->headingTrue);
        winVarManager->setVar(SIM_VAR_PLANE_LATITUDE, data->latitude);
        winVarManager->setVar(SIM_VAR_PLANE_LONGITUDE, data->longitude);
        winVarManager->setVar(SIM_VAR_NAV_DME_1, data->navDME1);
        winVarManager->setVar(SIM_VAR_NAV_DME_2, data->navDME2);
        winVarManager->setVar(SIM_VAR_SIMULATION_RATE, data->simRate);
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

static void runner() {
  auto prev = std::chrono::steady_clock::now();

  while (!__exit) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::nanoseconds delta = now - prev;
    prev = now;

    // If SC, call and process data;
    if (simConnect != NULL) {
      handleSimConnect();
    }

    {
      std::lock_guard<std::mutex> guard(lock);
      double simRate = 1;
      winVarManager->getVar(SIM_VAR_SIMULATION_RATE, simRate);
      ins->update(delta.count() * 1e-9 * simRate);
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
    std::cout << "Mode Knob: Arrow left/right" << std::endl;
    std::cout << "Data Knob: Arrow up/down" << std::endl;
    std::cout << "WPT sel  : Numpad +/-" << std::endl;
    std::cout << "INSERT   : Numpad enter" << std::endl;
    std::cout << "TEST     : T" << std::endl;
    std::cout << "DME LL   : L" << std::endl;
    std::cout << "DME FREQ : F" << std::endl;
    std::cout << "CLEAR    : DEL" << std::endl;
    std::cout << "WPT CHG  : W" << std::endl;
    std::cout << "HOLD     : H" << std::endl;
    std::cout << "AUTO/MAN : A" << std::endl;
    std::cout << "INST ALIG: I" << std::endl << std::endl;

    std::cout << "dT was " << delta.count() * 1e-6 << "ms" << std::endl;
  }
}

static void setupSimConnect() {
  HRESULT hr;

  hr = SimConnect_Open(&simConnect, "civaWin", NULL, 0, NULL, 0);
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

  SimConnect_RequestDataOnSimObject(simConnect, REQUEST_DEFINITIONS_DATA, DATA_DEFINITIONS_DATA, SIMCONNECT_OBJECT_ID_USER,
                                    SIMCONNECT_PERIOD_VISUAL_FRAME);
  // TODO:SC init
}

int main() {
  setupSimConnect();

  winVarManager = std::make_unique<WinVarManager>();

  ins = std::make_unique<INSContainer>(*winVarManager, UNIT_COUNT::THREE, UNIT_HAS_DME::BOTH, "");

  INSThread = std::thread(runner);

  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  INPUT_RECORD inp;
  DWORD num_of_events;

  // Mainloop
  while (!__exit) {
    ReadConsoleInput(hIn, &inp, 1, &num_of_events);

    std::lock_guard<std::mutex> guard(lock);

    ins->handleEvent([inp](auto unit1, auto unit2, auto unit3) {
      switch (inp.EventType) {
        case KEY_EVENT: {
          if (!inp.Event.KeyEvent.bKeyDown) {
            if (inp.Event.KeyEvent.wVirtualKeyCode == 'T') {
              unit1->handleTestButtonState(false);
              if (unit2) unit2->handleTestButtonState(false);
              if (unit3) unit3->handleTestButtonState(false);
            }

            break;
          }

          switch (inp.Event.KeyEvent.wVirtualKeyCode) {
            case VK_UP:
              unit1->incDataSelectorPos();
              if (unit2) unit2->incDataSelectorPos();
              if (unit3) unit3->incDataSelectorPos();
              break;
            case VK_DOWN:
              unit1->decDataSelectorPos();
              if (unit2) unit2->decDataSelectorPos();
              if (unit3) unit3->decDataSelectorPos();
              break;
            case VK_LEFT:
              unit1->decModeSelectorPos();
              if (unit2) unit2->decModeSelectorPos();
              if (unit3) unit3->decModeSelectorPos();
              break;
            case VK_RIGHT:
              unit1->incModeSelectorPos();
              if (unit2) unit2->incModeSelectorPos();
              if (unit3) unit3->incModeSelectorPos();
              break;
            case VK_NUMPAD0:
              unit1->handleNumeric(0);
              if (unit2) unit2->handleNumeric(0);
              if (unit3) unit3->handleNumeric(0);
              break;
            case VK_NUMPAD1:
              unit1->handleNumeric(1);
              if (unit2) unit2->handleNumeric(1);
              if (unit3) unit3->handleNumeric(1);
              break;
            case VK_NUMPAD2:
              unit1->handleNumeric(2);
              if (unit2) unit2->handleNumeric(2);
              if (unit3) unit3->handleNumeric(2);
              break;
            case VK_NUMPAD3:
              unit1->handleNumeric(3);
              if (unit2) unit2->handleNumeric(3);
              if (unit3) unit3->handleNumeric(3);
              break;
            case VK_NUMPAD4:
              unit1->handleNumeric(4);
              if (unit2) unit2->handleNumeric(4);
              if (unit3) unit3->handleNumeric(4);
              break;
            case VK_NUMPAD5:
              unit1->handleNumeric(5);
              if (unit2) unit2->handleNumeric(5);
              if (unit3) unit3->handleNumeric(5);
              break;
            case VK_NUMPAD6:
              unit1->handleNumeric(6);
              if (unit2) unit2->handleNumeric(6);
              if (unit3) unit3->handleNumeric(6);
              break;
            case VK_NUMPAD7:
              unit1->handleNumeric(7);
              if (unit2) unit2->handleNumeric(7);
              if (unit3) unit3->handleNumeric(7);
              break;
            case VK_NUMPAD8:
              unit1->handleNumeric(8);
              if (unit2) unit2->handleNumeric(8);
              if (unit3) unit3->handleNumeric(8);
              break;
            case VK_NUMPAD9:
              unit1->handleNumeric(9);
              if (unit2) unit2->handleNumeric(9);
              if (unit3) unit3->handleNumeric(9);
              break;
            case VK_RETURN:
              unit1->handleInsert();
              if (unit2) unit2->handleInsert();
              if (unit3) unit3->handleInsert();
              break;
            case VK_ADD:
              unit1->incWaypointSelectorPos();
              if (unit2) unit2->incWaypointSelectorPos();
              if (unit3) unit3->incWaypointSelectorPos();
              break;
            case VK_SUBTRACT:
              unit1->decWaypointSelectorPos();
              if (unit2) unit2->decWaypointSelectorPos();
              if (unit3) unit3->decWaypointSelectorPos();
              break;
            case VK_DELETE:
              unit1->handleClear();
              if (unit2) unit2->handleClear();
              if (unit3) unit3->handleClear();
              break;
            case 'T':
              unit1->handleTestButtonState(true);
              if (unit2) unit2->handleTestButtonState(true);
              if (unit3) unit3->handleTestButtonState(true);
              Sleep(200); // DEBOUNCE;
              break;
            case 'L':
            case 'F':
              unit1->handleDMEModeEntry((const uint8_t)inp.Event.KeyEvent.wVirtualKeyCode);
              if (unit2) unit2->handleDMEModeEntry((const uint8_t)inp.Event.KeyEvent.wVirtualKeyCode);
              if (unit3) unit3->handleDMEModeEntry((const uint8_t)inp.Event.KeyEvent.wVirtualKeyCode);
              break;
            case 'W':
              unit1->handleWaypointChange();
              if (unit2) unit2->handleWaypointChange();
              if (unit3) unit3->handleWaypointChange();
              break;
            case 'H':
              unit1->handleHoldButton();
              if (unit2) unit2->handleHoldButton();
              if (unit3) unit3->handleHoldButton();
              break;
            case 'A':
              unit1->handleAutoMan();
              if (unit2) unit2->handleAutoMan();
              if (unit3) unit3->handleAutoMan();
              break;
            case 'I':
              unit1->handleInstantAlign();
              if (unit2) unit2->handleInstantAlign();
              if (unit3) unit3->handleInstantAlign();
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
