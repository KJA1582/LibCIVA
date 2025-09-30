#include "ins.h"

static void formatPos(DISPLAY &display, double lat, double lon) noexcept {
  if (lat == 999) {
    display.characters.N = display.characters.S = false;
    lat = 0;
  }
  else if (lat < 0) {
    display.characters.N = false;
    display.characters.S = true;
    lat = -1 * lat;
  }
  else {
    display.characters.N = true;
    display.characters.S = false;
  }

  if (lon == 999) {
    display.characters.E = display.characters.W = false;
    lon = 0;
  }
  else if (lon < 0) {
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
  double minutes = round((lat - (int)lat) * 600);
  display.characters.LEFT_3 = (int)minutes / 100;
  display.characters.LEFT_4 = (int)minutes / 10 - display.characters.LEFT_3 * 10;
  display.characters.LEFT_5 = minutes - display.characters.LEFT_3 * 100 -
    display.characters.LEFT_4 * 10;

  display.characters.RIGHT_1 = (int)lon / 100;
  display.characters.RIGHT_2 = (int)lon / 10 - display.characters.RIGHT_1 * 10;
  display.characters.RIGHT_3 = (int)lon - display.characters.RIGHT_1 * 100 -
    display.characters.RIGHT_2 * 10;
  minutes = round((lon - (int)lon) * 600);
  display.characters.RIGHT_4 = (int)minutes / 100;
  display.characters.RIGHT_5 = (int)minutes / 10 - display.characters.RIGHT_4 * 10;
  display.characters.RIGHT_6 = minutes - display.characters.RIGHT_4 * 100 -
    display.characters.RIGHT_5 * 10;
}

void INS::display() const noexcept {
  DISPLAY display = getDisplay();
  DATA_SELECTOR_POS dataPos = getDataSelectorPos();
  INSERT_MODE insertMode = getInsertMode();
  WPT_SELECTOR_POS wptSelector = getWPTSelectorPos();

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

      formatPos(display, lat, lon);

      setDisplay(display);

      break;
    }
    case DATA_SELECTOR_POS::WPT:
    {
      if (insertMode != INSERT_MODE::INV) break;

      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEC_2 = true;

      double lat = getWPTPosLat(wptSelector);
      double lon = getWPTPosLon(wptSelector);

      formatPos(display, lat, lon);

      setDisplay(display);

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