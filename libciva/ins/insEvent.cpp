#include "ins.h"

static void readLat(DISPLAY &display, char &read, char value) noexcept {
  display.characters.LEFT_1 = display.characters.LEFT_2;
  display.characters.LEFT_2 = display.characters.LEFT_3;
  display.characters.LEFT_3 = display.characters.LEFT_4;
  display.characters.LEFT_4 = display.characters.LEFT_5;
  display.characters.LEFT_5 = value;

  read++;
}
static void readLon(DISPLAY &display, char &read, char value) noexcept {
  display.characters.RIGHT_1 = display.characters.RIGHT_2;
  display.characters.RIGHT_2 = display.characters.RIGHT_3;
  display.characters.RIGHT_3 = display.characters.RIGHT_4;
  display.characters.RIGHT_4 = display.characters.RIGHT_5;
  display.characters.RIGHT_5 = display.characters.RIGHT_6;
  display.characters.RIGHT_6 = value;

  read++;
}

static void clearLat(DISPLAY &display) noexcept {
  display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
    display.characters.LEFT_4 = display.characters.LEFT_5 = 0;
}
static void clearLon(DISPLAY &display) noexcept {
  display.characters.RIGHT_1 = display.characters.RIGHT_2 = display.characters.RIGHT_3 =
    display.characters.RIGHT_4 = display.characters.RIGHT_5 = display.characters.RIGHT_6 = 0;
}

static void startLatN(DISPLAY &display, INDICATORS &indicators, char &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearLat(display);

  display.characters.N = true;
  display.characters.S = false;
}
static void startLatS(DISPLAY &display, INDICATORS &indicators, char &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearLat(display);

  display.characters.N = false;
  display.characters.S = true;
}
static void startLonE(DISPLAY &display, INDICATORS &indicators, char &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearLon(display);

  display.characters.E = true;
  display.characters.W = false;
}
static void startLonW(DISPLAY &display, INDICATORS &indicators, char &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearLon(display);

  display.characters.E = false;
  display.characters.W = true;
}

static double convertLat(DISPLAY &display) noexcept {
  double latD = display.characters.LEFT_1 * 10 + display.characters.LEFT_2;
  double latM = display.characters.LEFT_3 * 10 + display.characters.LEFT_4 +
    (double)display.characters.LEFT_5 / 10.0;
  return (display.characters.N ? 1 : -1) * (latD + latM / 60.0);
}
static double convertLon(DISPLAY &display) noexcept {
  double lonD = display.characters.RIGHT_1 * 100 + display.characters.RIGHT_2 * 10 +
    display.characters.RIGHT_3;
  double lonM = display.characters.RIGHT_4 * 10 + display.characters.RIGHT_5 +
    (double)display.characters.RIGHT_6 / 10.0;
  return (display.characters.E ? 1 : -1) * (lonD + lonM / 60.0);
}

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
  WPT_SELECTOR_POS wptSelector = getWPTSelectorPos();

  if (state <= INS_STATE::OFF || state >= INS_STATE::ATT) return;

  switch (pos) {
    case DATA_SELECTOR_POS::POS: {
      if (value == 2 && insertMode == INSERT_MODE::INV) {
        setInsertMode(INSERT_MODE::POS_LAT);
        startLatS(display, indicators, read);
      }
      else if (value == 8 && insertMode == INSERT_MODE::INV) {
        setInsertMode(INSERT_MODE::POS_LAT);
        startLatN(display, indicators, read);
      }
      else if (insertMode == INSERT_MODE::POS_LAT && read < 5) {
        readLat(display, read, value);
      }
      else if (insertMode == INSERT_MODE::PRE_POS_LON && value == 4) {
        setInsertMode(INSERT_MODE::POS_LON);
        startLonW(display, indicators, read);
      }
      else if (insertMode == INSERT_MODE::PRE_POS_LON && value == 6) {
        setInsertMode(INSERT_MODE::POS_LON);
        startLonE(display, indicators, read);
      }
      else if (insertMode == INSERT_MODE::POS_LON && read < 6) {
        readLon(display, read, value);
      }

      break;
    }
    case DATA_SELECTOR_POS::WPT: {
      if (wptSelector < WPT_SELECTOR_POS::WPT_1) break;
      else if (value == 2 && insertMode == INSERT_MODE::INV) {
        setInsertMode(INSERT_MODE::WPT_LAT);
        startLatS(display, indicators, read);
      }
      else if (value == 8 && insertMode == INSERT_MODE::INV) {
        setInsertMode(INSERT_MODE::WPT_LAT);
        startLatN(display, indicators, read);
      }
      else if (insertMode == INSERT_MODE::WPT_LAT && read < 5) {
        readLat(display, read, value);
      }
      else if (insertMode == INSERT_MODE::INV && value == 4) {
        setInsertMode(INSERT_MODE::WPT_LON);
        startLonW(display, indicators, read);
      }
      else if (insertMode == INSERT_MODE::INV && value == 6) {
        setInsertMode(INSERT_MODE::WPT_LON);
        startLonE(display, indicators, read);
      }
      else if (insertMode == INSERT_MODE::WPT_LON && read < 6) {
        readLon(display, read, value);
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
  WPT_SELECTOR_POS wptSelector = getWPTSelectorPos();

  switch (insertMode) {
    case INSERT_MODE::POS_LAT: {
      setInsertMode(INSERT_MODE::PRE_POS_LON);

      break;
    }
    case INSERT_MODE::POS_LON: {
      indicators.indicator.INSERT = false;

      setDisplayPosLat(convertLat(display));
      setDisplayPosLon(convertLon(display));

      setInsertMode(INSERT_MODE::INV);
      setIndicators(indicators);

      break;
    }
    case INSERT_MODE::WPT_LAT: {
      indicators.indicator.INSERT = false;

      setWPTPosLat(convertLat(display), wptSelector);

      if (getWPTPosLon(wptSelector) == 999) {
        setWPTPosLon(0, wptSelector);
      }

      setInsertMode(INSERT_MODE::INV);
      setIndicators(indicators);

      break;
    }
    case INSERT_MODE::WPT_LON: {
      indicators.indicator.INSERT = false;

      setWPTPosLon(convertLon(display), wptSelector);

      if (getWPTPosLat(wptSelector) == 999) {
        setWPTPosLat(0, wptSelector);
      }

      setInsertMode(INSERT_MODE::INV);
      setIndicators(indicators);

      break;
    }
  }
}

void INS::incWPTSelectorPos() const noexcept {
  WPT_SELECTOR_POS state = getWPTSelectorPos();
  if (state != WPT_SELECTOR_POS::INV) {
    setWPTSelectorPos((WPT_SELECTOR_POS)(((int)state + 1) % 10));
  }
}
void INS::decWPTSelectorPos() const noexcept {
  WPT_SELECTOR_POS state = getWPTSelectorPos();
  if (state != WPT_SELECTOR_POS::INV) {
    setWPTSelectorPos(state == WPT_SELECTOR_POS::WPT_0 ? WPT_SELECTOR_POS::WPT_9 :
                      (WPT_SELECTOR_POS)((int)state - 1));
  }
}
