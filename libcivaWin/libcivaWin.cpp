// libciva.cpp : Defines the entry point for the application.
//

#include "libcivaWin.h"

std::unique_ptr<WinVarManager> winVarManager;
std::unique_ptr<INS> unit1;

std::thread unit1Thread;
std::atomic<bool> __exit;

static void runner() {
  auto prev = std::chrono::steady_clock::now();

  while (!__exit) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::nanoseconds delta = now - prev;
    prev = now;

    // FIXME: 100 times as fast as IRL (-9)
    unit1->update(delta.count() * 1e-7);

    HANDLE handle;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordinates = {};
    SetConsoleCursorPosition(handle, coordinates);

    winVarManager->dump();
    std::cout << "dT was " << delta.count() * 1e-6 << "ms" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int main() {
  winVarManager = std::make_unique<WinVarManager>();
  winVarManager->setVar(SIM_VAR_AMBIENT_TEMPERATURE, 20);

  unit1 = std::make_unique<INS>(*winVarManager, "UNIT_1", WORK_DIR);

  unit1Thread = std::thread(runner);

  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  INPUT_RECORD inp;
  DWORD num_of_events;

  // Mainloop
  while (!__exit) {
    ReadConsoleInput(hIn, &inp, 1, &num_of_events);

    switch (inp.EventType) {
      case KEY_EVENT:
        if (!inp.Event.KeyEvent.bKeyDown) break;

        switch (inp.Event.KeyEvent.wVirtualKeyCode) {
          case VK_UP:
            unit1->incDataSelectorPos();
            break;
          case VK_DOWN:
            unit1->decDataSelectorPos();
            break;
          case VK_LEFT:
            unit1->decModeSelectorPos();
            break;
          case VK_RIGHT:
            unit1->incModeSelectorPos();
            break;
          case VK_NUMPAD0:
            unit1->handleNumeric(0);
            break;
          case VK_NUMPAD1:
            unit1->handleNumeric(1);
            break;
          case VK_NUMPAD2:
            unit1->handleNumeric(2);
            break;
          case VK_NUMPAD3:
            unit1->handleNumeric(3);
            break;
          case VK_NUMPAD4:
            unit1->handleNumeric(4);
            break;
          case VK_NUMPAD5:
            unit1->handleNumeric(5);
            break;
          case VK_NUMPAD6:
            unit1->handleNumeric(6);
            break;
          case VK_NUMPAD7:
            unit1->handleNumeric(7);
            break;
          case VK_NUMPAD8:
            unit1->handleNumeric(8);
            break;
          case VK_NUMPAD9:
            unit1->handleNumeric(9);
            break;
          case VK_RETURN:
            unit1->handleInsert();
            break;
          case VK_ESCAPE:
            __exit = true;
            break;
        }
        break;
    }

    Sleep(20);
  }

  unit1Thread.join();

  return 0;
}
