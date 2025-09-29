#include "ins.h"

void INS::display() const noexcept {
  DISPLAY display = getDisplay();
  DATA_SELECTOR_POS dataPos = getDataSelectorPos();
  INSERT_MODE insertMode = getInsertMode();

  switch (dataPos) {
    case DATA_SELECTOR_POS::TKGS: {
      break;
    }
    case DATA_SELECTOR_POS::HDGDA: {
      break;
    }
    case DATA_SELECTOR_POS::XTKTKE: {
      break;
    }
    case DATA_SELECTOR_POS::POS: {
      if (insertMode != INSERT_MODE::INV) break;

      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEC_2 = true;

      double lat = getDisplayPosLat();
      double lon = getDisplayPosLon();

      if (lat < 0) {
        display.characters.N = false;
        display.characters.S = true;
        lat = -1 * lat;
      }
      else {
        display.characters.N = true;
        display.characters.S = false;
      }
      if (lon < 0) {
        display.characters.E = false;
        display.characters.W = true;
        lon = -1 * lon;
      }
      else {
        display.characters.E = true;
        display.characters.W = false;
      }

      display.characters.LEFT_1 = (int)lat / 10;
      display.characters.LEFT_2 = (int)lat - display.characters.LEFT_1 * 10;
      double minutes = (lat - (int)lat) * 60;
      display.characters.LEFT_3 = (int)minutes / 10;
      display.characters.LEFT_4 = (int)minutes - display.characters.LEFT_3 * 10;
      display.characters.LEFT_5 = round((minutes * 10) - display.characters.LEFT_3 * 100 - 
        display.characters.LEFT_4 * 10);

      display.characters.RIGHT_1 = (int)lon / 100;
      display.characters.RIGHT_2 = (int)lon / 10 - display.characters.RIGHT_1 * 10;
      display.characters.RIGHT_3 = (int)lon - display.characters.RIGHT_1 * 100 -
        display.characters.RIGHT_2 * 10;
      minutes = (lon - (int)lon) * 60;
      display.characters.RIGHT_4 = (int)minutes / 10;
      display.characters.RIGHT_5 = (int)minutes - display.characters.RIGHT_4 * 10;
      display.characters.RIGHT_6 = round((minutes * 10) - display.characters.RIGHT_4 * 100 -
        display.characters.RIGHT_5 * 10);

      setDisplay(display);

      break;
    }
    case DATA_SELECTOR_POS::WPT: {
      break;
    }
    case DATA_SELECTOR_POS::DISTIME: {
      break;
    }
    case DATA_SELECTOR_POS::WIND: {
      break;
    }
    case DATA_SELECTOR_POS::DSRTKSTS: {
      break;
    }
  }
}