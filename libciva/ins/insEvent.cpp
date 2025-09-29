#include "ins.h"

void INS::incDataSelectorPos() const noexcept {
  DATA_SELECTOR_POS state = getDataSelectorPos();
  if (state != DATA_SELECTOR_POS::INV && state != DATA_SELECTOR_POS::DSRTKSTS) {
    setDataSelectorPos((DATA_SELECTOR_POS)((int)state + 1));
  }
}
void INS::decDataSelectorPos() const noexcept {
  DATA_SELECTOR_POS state = getDataSelectorPos();
  if (state != DATA_SELECTOR_POS::INV && state != DATA_SELECTOR_POS::TKGS) {
    setDataSelectorPos((DATA_SELECTOR_POS)((int)state - 1));
  }
}

void INS::incModeSelectorPos() const noexcept {
  MODE_SELECTOR_POS state = getModeSelectorPos();
  if (state != MODE_SELECTOR_POS::INV && state != MODE_SELECTOR_POS::ATT) {
    setModeSelectorPos((MODE_SELECTOR_POS)((int)state + 1));
  }
}
void INS::decModeSelectorPos() const noexcept {
  MODE_SELECTOR_POS state = getModeSelectorPos();
  if (state != MODE_SELECTOR_POS::INV && state != MODE_SELECTOR_POS::OFF) {
    setModeSelectorPos((MODE_SELECTOR_POS)((int)state - 1));
  }
}

void INS::handleNumeric(char value) const noexcept {
  static char read = 0;

  DATA_SELECTOR_POS pos = getDataSelectorPos();
  INS_STATE state = getINSState();
  DISPLAY display = getDisplay();
  INDICATORS indicators = getIndicators();
  INSERT_MODE insertMode = getInsertMode();

  if (state <= INS_STATE::OFF || state >= INS_STATE::ATT) return;

  switch (pos) {
    case DATA_SELECTOR_POS::POS: {
      if (value == 2 && insertMode == INSERT_MODE::INV) {
        setInsertMode(INSERT_MODE::POS_LAT);

        indicators.indicator.INSERT = true;
        read = 0;

        display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
          display.characters.LEFT_4 = display.characters.LEFT_5 = 0;
        display.characters.N = false;
        display.characters.S = true;
      }
      else if (value == 8 && insertMode == INSERT_MODE::INV) {
        setInsertMode(INSERT_MODE::POS_LAT);

        indicators.indicator.INSERT = true;
        read = 0;

        display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
          display.characters.LEFT_4 = display.characters.LEFT_5 = 0;
        display.characters.N = true;
        display.characters.S = false;
      }
      else if (insertMode == INSERT_MODE::POS_LAT && read < 5) {
        display.characters.LEFT_1 = display.characters.LEFT_2;
        display.characters.LEFT_2 = display.characters.LEFT_3;
        display.characters.LEFT_3 = display.characters.LEFT_4;
        display.characters.LEFT_4 = display.characters.LEFT_5;
        display.characters.LEFT_5 = value;

        read++;
      }
      else if (insertMode == INSERT_MODE::PRE_POS_LON && value == 4) {
        setInsertMode(INSERT_MODE::POS_LON);

        read = 0;

        display.characters.RIGHT_1 = display.characters.RIGHT_2 = display.characters.RIGHT_3 =
          display.characters.RIGHT_4 = display.characters.RIGHT_5 = display.characters.RIGHT_6 = 0;
        display.characters.E = false;
        display.characters.W = true;
      }
      else if (insertMode == INSERT_MODE::PRE_POS_LON && value == 6) {
        setInsertMode(INSERT_MODE::POS_LON);

        read = 0;

        display.characters.RIGHT_1 = display.characters.RIGHT_2 = display.characters.RIGHT_3 =
          display.characters.RIGHT_4 = display.characters.RIGHT_5 = display.characters.RIGHT_6 = 0;
        display.characters.E = true;
        display.characters.W = false;
      }
      else if (insertMode == INSERT_MODE::POS_LON && read < 6) {
        display.characters.RIGHT_1 = display.characters.RIGHT_2;
        display.characters.RIGHT_2 = display.characters.RIGHT_3;
        display.characters.RIGHT_3 = display.characters.RIGHT_4;
        display.characters.RIGHT_4 = display.characters.RIGHT_5;
        display.characters.RIGHT_5 = display.characters.RIGHT_6;
        display.characters.RIGHT_6 = value;

        read++;
      }

      break;
    }
  }

  setDisplay(display);
  setIndicators(indicators);
}

void INS::handleInsert() const noexcept {
  DISPLAY display = getDisplay();
  INDICATORS indicators = getIndicators();
  INSERT_MODE insertMode = getInsertMode();

  switch (insertMode) {
    case INSERT_MODE::POS_LAT: {
      setInsertMode(INSERT_MODE::PRE_POS_LON);

      break;
    }
    case INSERT_MODE::POS_LON: {
      indicators.indicator.INSERT = false;

      double latD = display.characters.LEFT_1 * 10 + display.characters.LEFT_2;
      double latM = display.characters.LEFT_3 * 10 + display.characters.LEFT_4 +
        (double)display.characters.LEFT_5 / 10.0;
      double lat = (display.characters.N ? 1 : -1) * (latD + latM / 60.0);

      double lonD = display.characters.RIGHT_1 * 100 + display.characters.RIGHT_2 * 10 +
        display.characters.RIGHT_3;
      double lonM = display.characters.RIGHT_4 * 10 + display.characters.RIGHT_5 +
        (double)display.characters.RIGHT_6 / 10.0;
      double lon = (display.characters.E ? 1 : -1) * (lonD + lonM / 60.0);

      setInsertMode(INSERT_MODE::INV);
      setIndicators(indicators);
      setDisplayPosLat(lat);
      setDisplayPosLon(lon);

      break;
    }
  }
}