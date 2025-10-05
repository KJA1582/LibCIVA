#include "ins.h"

static void readLat(DISPLAY &display, uint8_t &read, const uint8_t value) noexcept {
  display.characters.LEFT_1 = display.characters.LEFT_2;
  display.characters.LEFT_2 = display.characters.LEFT_3;
  display.characters.LEFT_3 = display.characters.LEFT_4;
  display.characters.LEFT_4 = display.characters.LEFT_5;
  display.characters.LEFT_5 = value;

  read++;
}
static void readLon(DISPLAY &display, uint8_t &read, const uint8_t value) noexcept {
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

static void startLatN(DISPLAY &display, INDICATORS &indicators, uint8_t &read) noexcept {
indicators.indicator.INSERT = true;
  read = 0;

  clearLat(display);

  display.characters.N = true;
  display.characters.S = false;
}
static void startLatS(DISPLAY &display, INDICATORS &indicators, uint8_t &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearLat(display);

  display.characters.N = false;
  display.characters.S = true;
}
static void startLonE(DISPLAY &display, INDICATORS &indicators, uint8_t &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearLon(display);

  display.characters.E = true;
  display.characters.W = false;
}
static void startLonW(DISPLAY &display, INDICATORS &indicators, uint8_t &read) noexcept {
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

void INS::incDataSelectorPos() noexcept {
  if (dataSelector != DATA_SELECTOR::DSRTKSTS) {
    dataSelector = (DATA_SELECTOR)((uint8_t)dataSelector + 1);
  }
}
void INS::incModeSelectorPos() noexcept {
  if (modeSelector != MODE_SELECTOR::ATT) {
    modeSelector = (MODE_SELECTOR)((uint8_t)modeSelector + 1);
  }
}
void INS::incWaypointSelectorPos() noexcept {
  if (waypointSelector < 9) {
    waypointSelector += 1;
  }
}

void INS::decDataSelectorPos() noexcept {
  if (dataSelector != DATA_SELECTOR::TKGS) {
    dataSelector = (DATA_SELECTOR)((uint8_t)dataSelector - 1);
  }
}
void INS::decWaypointSelectorPos() noexcept {
  if (waypointSelector > 0) {
    waypointSelector -= 1;
  }
}
void INS::decModeSelectorPos() noexcept {
  if (modeSelector != MODE_SELECTOR::OFF) {
    modeSelector = (MODE_SELECTOR)((uint8_t)modeSelector - 1);
  }
}

void INS::handleNumeric(const uint8_t value) noexcept {
  // cannot enter anything when OFF or ATT
  if (state <= INS_STATE::OFF || state >= INS_STATE::ATT) return;

  switch (dataSelector) {
    case DATA_SELECTOR::POS: {
      if (value == 2 && insertMode == INSERT_MODE::INV) {
        insertMode = INSERT_MODE::POS_LAT;
        startLatS(display, indicators, charactersRead);
      }
      else if (value == 8 && insertMode == INSERT_MODE::INV) {
        insertMode = INSERT_MODE::POS_LAT;
        startLatN(display, indicators, charactersRead);
      }
      else if (insertMode == INSERT_MODE::POS_LAT && charactersRead < 5) {
        readLat(display, charactersRead, value);
      }
      else if (insertMode == INSERT_MODE::PRE_POS_LON && value == 4) {
        insertMode = INSERT_MODE::POS_LON;
        startLonW(display, indicators, charactersRead);
      }
      else if (insertMode == INSERT_MODE::PRE_POS_LON && value == 6) {
        insertMode = INSERT_MODE::POS_LON;
        startLonE(display, indicators, charactersRead);
      }
      else if (insertMode == INSERT_MODE::POS_LON && charactersRead < 6) {
        readLon(display, charactersRead, value);
      }

      break;
    }
    case DATA_SELECTOR::WPT: {
      // Waypoint zero is read only for C/DU
      if (waypointSelector < 1) break;
      else if (value == 2 && insertMode == INSERT_MODE::INV) {
        insertMode = INSERT_MODE::WPT_LAT;
        startLatS(display, indicators, charactersRead);
      }
      else if (value == 8 && insertMode == INSERT_MODE::INV) {
        insertMode = INSERT_MODE::WPT_LAT;
        startLatN(display, indicators, charactersRead);
      }
      else if (insertMode == INSERT_MODE::WPT_LAT && charactersRead < 5) {
        readLat(display, charactersRead, value);
      }
      else if (insertMode == INSERT_MODE::INV && value == 4) {
        insertMode = INSERT_MODE::WPT_LON;
        startLonW(display, indicators, charactersRead);
      }
      else if (insertMode == INSERT_MODE::INV && value == 6) {
        insertMode = INSERT_MODE::WPT_LON;
        startLonE(display, indicators, charactersRead);
      }
      else if (insertMode == INSERT_MODE::WPT_LON && charactersRead < 6) {
        readLon(display, charactersRead, value);
      }

      break;
    }
    case DATA_SELECTOR::DSRTKSTS: {
      if (state != INS_STATE::NAV && insertMode == INSERT_MODE::INV) break;

      insertMode = INSERT_MODE::PERFORMANCE_INDEX;

      displayPerformanceIndex = value;

      break;
    }
  }
}

void INS::handleInsert() noexcept {
  // Can't insert in OFF or ATT
  if (state == INS_STATE::OFF || state == INS_STATE::ATT) return;

  switch (insertMode) {
    case INSERT_MODE::POS_LAT: {
      if (dataSelector != DATA_SELECTOR::POS) break;

      displayPosition.latitude = convertLat(display);

      insertMode = INSERT_MODE::PRE_POS_LON;

      break;
    }
    case INSERT_MODE::PRE_POS_LON:
    case INSERT_MODE::POS_LON: {
      if (dataSelector != DATA_SELECTOR::POS) break;

      indicators.indicator.INSERT = false;

      displayPosition.longitude = convertLon(display);
      if (!displayPosition.isValid()) break; // Essential get the user stuck here

      if (state == INS_STATE::NAV) {
        // FIXME: Disallow in NAV if not in HOLD mode. Also move to HOLD mode exit
        if (displayPosition.distanceTo(currentINSPosition) > MAX_DEV) {
          addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A02_49);
          indicators.indicator.WARN = true;
        }

        currentINSPosition = displayPosition;
      }
      // In ALIGN
      else if (state == INS_STATE::ALIGN) {
        // MODE 5 or less, trigger 02-63 and denie nav entry by resetting to mode 6
        if (alignSubmode < ALIGN_SUBMODE::MODE_6) {
          addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A02_63);
          indicators.indicator.WARN = true;
          indicators.indicator.READY_NAV = false;
          alignSubmode = ALIGN_SUBMODE::MODE_6;
        }
        // MODE 6, trigger 04-41, reset to MODE 6, can continue
        else if (alignSubmode == ALIGN_SUBMODE::MODE_6) {
          addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A04_41);
          indicators.indicator.WARN = true;
          alignSubmode = ALIGN_SUBMODE::MODE_6;
        }
        // Others, trigger 04-41 if >76nmi from last
        else if (displayPosition.distanceTo(config.getLastINSPosisiton()) > MAX_RAMP_DEV) {
          addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A04_41);
          indicators.indicator.WARN = true;
        }

        currentINSPosition = initialINSPosition = displayPosition;
      }
      // Since OFF and ATT are early abort, this is STBY only
      // Tigger 04-41 if >76nmi from last
      else {
        if (displayPosition.distanceTo(config.getLastINSPosisiton()) > MAX_RAMP_DEV) {
          addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A04_41);
          indicators.indicator.WARN = true;
        }

        currentINSPosition = initialINSPosition = displayPosition;
      }

      insertMode = INSERT_MODE::INV;

      break;
    }
    case INSERT_MODE::WPT_LAT: {
      if (dataSelector != DATA_SELECTOR::WPT) break;

      indicators.indicator.INSERT = false;

      double lat = convertLat(display);
      if (lat > 90 || lat < -90) break;

      waypoints[waypointSelector].latitude = lat;

      if (waypoints[waypointSelector].longitude == 999) {
        waypoints[waypointSelector].longitude = 0;
      }

      insertMode = INSERT_MODE::INV;

      break;
    }
    case INSERT_MODE::WPT_LON: {
      if (dataSelector != DATA_SELECTOR::WPT) break;

      indicators.indicator.INSERT = false;

      double lon = convertLon(display);
      if (lon > 180 || lon < -180) break;

      waypoints[waypointSelector].longitude = lon;

      if (waypoints[waypointSelector].latitude == 999) {
        waypoints[waypointSelector].latitude = 0;
      }

      insertMode = INSERT_MODE::INV;

      break;
    }
    case INSERT_MODE::PERFORMANCE_INDEX: {
      indicators.indicator.INSERT = false;

      // Erradication
      if (displayPerformanceIndex == 1) {
        activePerformanceIndex = 5;
        currentINSPosition = initialINSPosition;
      }
      // Aided
      else if (displayPerformanceIndex == 4) {
        activePerformanceIndex = 4;
      }
      // Unaided
      else {
        activePerformanceIndex = 5;
      }

      insertMode = INSERT_MODE::INV;

      break;
    }
  }
}

void INS::handleTestButtonState(const bool state) noexcept {
  if (hasActionMalfunctionCode()) {
    // Pressed
    if (state) {
      if (mafunctionCodeDisplayed) {
        if (getCurrentActionMalfunctionCode() == ACTION_MALFUNCTION_CODE::A04_41) {
          if (removeCurrentActionMalfunctionCode()) {
            indicators.indicator.WARN = false;
          }

          config.setLastINSPosition(currentINSPosition);
        }
        else {
          incCurrentActionMalfunctionCode();
        }
      }

      mafunctionCodeDisplayed = !mafunctionCodeDisplayed;
    }
  }
  else {
    if (state) {
      if (!inTestMode) {
        previousIndicators = indicators;
      }
      inTestMode = true;
    }
    else {
      indicators = previousIndicators;
      previousIndicators = { 0 };
      inTestMode = false;
    }
  }
}