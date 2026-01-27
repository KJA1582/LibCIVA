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

static void readAlt(DISPLAY &display, uint8_t &read, const uint8_t value) noexcept {
  if (display.characters.LEFT_5 > 0) {
    display.characters.LEFT_4 = display.characters.LEFT_5;
  }
  display.characters.LEFT_5 = value;

  read++;
}

static void readFreq(DISPLAY &display, uint8_t &read, const uint8_t value) noexcept {
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

static void clearAlt(DISPLAY &display) noexcept {
  display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
    display.characters.LEFT_4 = DISPLAY_CHAR_BLANK;
  display.characters.LEFT_5 = 0;
}

static void clearFreq(DISPLAY &display) noexcept {
  display.characters.RIGHT_1 = DISPLAY_CHAR_BLANK;
  display.characters.RIGHT_2 = display.characters.RIGHT_3 =
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

static void startAlt(DISPLAY &display, INDICATORS &indicators, uint8_t &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearAlt(display);

  display.characters.N = display.characters.S = false;
}

static void startFreq(DISPLAY &display, INDICATORS &indicators, uint8_t &read) noexcept {
  indicators.indicator.INSERT = true;
  read = 0;

  clearFreq(display);

  display.characters.E = display.characters.W = false;
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
    if (dmeMode != DME_MODE::INV && dataSelector == DATA_SELECTOR::DISTIME) {
      dmeMode = DME_MODE::INV;
      insertMode = INSERT_MODE::INV;
      indicators.indicator.WAYPOINT_CHANGE = false;
      dataSelector = (DATA_SELECTOR)((uint8_t)dataSelector + 1);

      return;
    }
    dataSelector = (DATA_SELECTOR)((uint8_t)dataSelector + 1);

    if (insertMode != INSERT_MODE::WPT_CHG_FROM && insertMode != INSERT_MODE::WPT_CHG_TO) {
      insertMode = INSERT_MODE::INV;
    }
    if (currentINSPosition.isValid() && insertMode != INSERT_MODE::WPT_CHG_FROM &&
        insertMode != INSERT_MODE::WPT_CHG_TO) {
      indicators.indicator.INSERT = false;
    }
  }
}

void INS::incModeSelectorPos() noexcept {
  if (modeSelector != MODE_SELECTOR::ATT) {
    modeSelector = (MODE_SELECTOR)((uint8_t)modeSelector + 1);
  }
}

void INS::incWaypointSelectorPos() noexcept {
  waypointSelector = (waypointSelector + 1) % 10;
}


void INS::decDataSelectorPos() noexcept {
  if (dataSelector != DATA_SELECTOR::TKGS) {
    if (dmeMode != DME_MODE::INV && dataSelector == DATA_SELECTOR::WPT) {
      dmeMode = DME_MODE::INV;
      insertMode = INSERT_MODE::INV;
      indicators.indicator.WAYPOINT_CHANGE = false;
      dataSelector = (DATA_SELECTOR)((uint8_t)dataSelector - 1);

      return;
    }
    dataSelector = (DATA_SELECTOR)((uint8_t)dataSelector - 1);

    if (insertMode != INSERT_MODE::WPT_CHG_FROM && insertMode != INSERT_MODE::WPT_CHG_TO) {
      insertMode = INSERT_MODE::INV;
    }
    if (currentINSPosition.isValid() && insertMode != INSERT_MODE::WPT_CHG_FROM &&
        insertMode != INSERT_MODE::WPT_CHG_TO) {
      indicators.indicator.INSERT = false;
    }
  }
}

void INS::decModeSelectorPos() noexcept {
  if (modeSelector != MODE_SELECTOR::OFF) {
    modeSelector = (MODE_SELECTOR)((uint8_t)modeSelector - 1);
  }
}

void INS::decWaypointSelectorPos() noexcept {
  if (waypointSelector > 0) {
    waypointSelector -= 1;
  }
  else {
    waypointSelector = 9;
  }
}


void INS::handleNumeric(const uint8_t value) noexcept {
  // cannot enter anything when OFF or ATT
  if (state <= INS_STATE::OFF || state >= INS_STATE::ATT) return;

  // WPT CHG
  if (insertMode == INSERT_MODE::WPT_CHG_FROM) {
    display.characters.FROM = value;
    insertMode = INSERT_MODE::WPT_CHG_TO;

    return;
  }
  else if (insertMode == INSERT_MODE::WPT_CHG_TO) {
    display.characters.TO = value;
    insertMode = INSERT_MODE::WPT_CHG_FROM;

    return;
  }

  switch (dataSelector) {
    case DATA_SELECTOR::POS: {
      if (state == INS_STATE::NAV && !inHoldMode) break;;

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
        if (dmeMode == DME_MODE::DME_FREQ) {
          insertMode = INSERT_MODE::DME_ALT;
          startAlt(display, indicators, charactersRead);
        }
        else {
          insertMode = INSERT_MODE::WPT_LAT;
          startLatS(display, indicators, charactersRead);
        }
      }
      else if (value == 8 && insertMode == INSERT_MODE::INV) {
        if (dmeMode == DME_MODE::DME_FREQ) {
          insertMode = INSERT_MODE::DME_ALT;
          startAlt(display, indicators, charactersRead);
        }
        else {
          insertMode = INSERT_MODE::WPT_LAT;
          startLatN(display, indicators, charactersRead);
        }
      }
      else if (insertMode == INSERT_MODE::WPT_LAT && charactersRead < 5) {
        readLat(display, charactersRead, value);
      }
      else if (insertMode == INSERT_MODE::DME_ALT && charactersRead < 2) {
        readAlt(display, charactersRead, value);
      }
      else if (insertMode == INSERT_MODE::INV && value == 4) {
        if (dmeMode == DME_MODE::DME_FREQ) {
          insertMode = INSERT_MODE::DME_FREQ;
          startFreq(display, indicators, charactersRead);
        }
        else {
          insertMode = INSERT_MODE::WPT_LON;
          startLonW(display, indicators, charactersRead);
        }
      }
      else if (insertMode == INSERT_MODE::INV && value == 6) {
        if (dmeMode == DME_MODE::DME_FREQ) {
          insertMode = INSERT_MODE::DME_FREQ;
          startFreq(display, indicators, charactersRead);
        }
        else {
          insertMode = INSERT_MODE::WPT_LON;
          startLonE(display, indicators, charactersRead);
        }
      }
      else if (insertMode == INSERT_MODE::WPT_LON && charactersRead < 6) {
        readLon(display, charactersRead, value);
      }
      else if (insertMode == INSERT_MODE::DME_FREQ && charactersRead < 5) {
        readFreq(display, charactersRead, value);
      }

      break;
    }
    case DATA_SELECTOR::DSRTKSTS: {
      if (state != INS_STATE::NAV && insertMode == INSERT_MODE::INV) break;

      indicators.indicator.INSERT = true;
      insertMode = INSERT_MODE::PERFORMANCE_INDEX;

      display.characters.RIGHT_6 = value;

      break;
    }
    default: {
      break;
    }
  }
}

void INS::handleInsert() noexcept {
  // Can't insert in OFF or ATT
  if (state == INS_STATE::OFF || state == INS_STATE::ATT) return;

  switch (insertMode) {
    case INSERT_MODE::INV: {
      break;
    }
    case INSERT_MODE::POS_LAT: {
      if (dataSelector != DATA_SELECTOR::POS) return;

      displayPosition.latitude = convertLat(display);

      insertMode = INSERT_MODE::PRE_POS_LON;
      return;
    }
    case INSERT_MODE::PRE_POS_LON:
    case INSERT_MODE::POS_LON: {
      if (dataSelector != DATA_SELECTOR::POS) return;

      displayPosition.longitude = convertLon(display);
      if (!displayPosition.isValid()) return; // Essential get the user stuck here

      if (state == INS_STATE::NAV && inHoldMode) {
        // Reject
        if (displayPosition.distanceTo(initialINSPosition) > MAX_DEV) {
          actionMalfunctionCodes.codes.A02_49 = true;
          advanceActionMalfunctionIndex();
          indicators.indicator.WARN = holdRequiresForce = true;
          break;
        }

        currentError = { 0, 0 };
        timeInNAV = 0;

        // Get delta of reference entered and what froze
        POSITION delta = holdPosition - displayPosition;
        // Set updated position to be w/e currently is + delta
        currentINSPosition = currentINSPosition + delta;
        updateSimPosDelta();
        indicators.indicator.HOLD = false;
        inHoldMode = false;
      }
      // In ALIGN
      else if (state == INS_STATE::ALIGN) {
        // MODE 5 or less, trigger 02-63 and deny nav entry by resetting to mode 6
        if (alignSubmode < ALIGN_SUBMODE::MODE_6) {
          actionMalfunctionCodes.codes.A02_63 = true;
          advanceActionMalfunctionIndex();
          indicators.indicator.WARN = true;
          indicators.indicator.READY_NAV = false;
          alignSubmode = ALIGN_SUBMODE::MODE_6;
        }
        // MODE 6, trigger 04-41, reset to MODE 6, can continue
        else if (alignSubmode == ALIGN_SUBMODE::MODE_6) {
          actionMalfunctionCodes.codes.A04_41 = true;
          advanceActionMalfunctionIndex();
          indicators.indicator.WARN = true;
          alignSubmode = ALIGN_SUBMODE::MODE_6;
        }
        // Others, trigger 04-41 if >76nmi from last
        else if (displayPosition.distanceTo(config.getLastINSPosisiton()) > MAX_RAMP_DEV) {
          actionMalfunctionCodes.codes.A04_41 = true;
          advanceActionMalfunctionIndex();
          indicators.indicator.WARN = true;
        }

        waypoints[0] = currentINSPosition = initialINSPosition = displayPosition;
      }
      // Since OFF and ATT are early abort, this is STBY only
      // Tigger 04-41 if >76nmi from last
      else {
        if (displayPosition.distanceTo(config.getLastINSPosisiton()) > MAX_RAMP_DEV) {
          actionMalfunctionCodes.codes.A04_41 = true;
          advanceActionMalfunctionIndex();
          indicators.indicator.WARN = true;
        }

        waypoints[0] = currentINSPosition = initialINSPosition = displayPosition;
      }

      break;
    }
    case INSERT_MODE::WPT_LAT: {
      if (dataSelector != DATA_SELECTOR::WPT) return;

      double lat = convertLat(display);
      if (lat > 90 || lat < -90) return;

      if (dmeMode == DME_MODE::DME_LL && waypointSelector >= 1) {
        DMEs[waypointSelector - 1].position.latitude = lat;

        if (activeDME == waypointSelector - 1) {
          activeDME = 0;
        }
      }
      else if (dmeMode == DME_MODE::INV) {
        waypoints[waypointSelector].latitude = lat;
      }

      break;
    }
    case INSERT_MODE::WPT_LON: {
      if (dataSelector != DATA_SELECTOR::WPT) return;

      double lon = convertLon(display);
      if (lon > 180 || lon < -180) return;

      if (dmeMode == DME_MODE::DME_LL && waypointSelector >= 1) {
        DMEs[waypointSelector - 1].position.longitude = lon;

        if (activeDME == waypointSelector - 1) {
          activeDME = 0;
        }
      }
      else if (dmeMode == DME_MODE::INV) {
        waypoints[waypointSelector].longitude = lon;
      }

      break;
    }
    case INSERT_MODE::DME_ALT: {
      if (dataSelector != DATA_SELECTOR::WPT && dmeMode != DME_MODE::DME_FREQ) return;
      if (waypointSelector == 0) break;

      uint8_t altT = display.characters.LEFT_4;
      uint8_t alt = (altT != DISPLAY_CHAR_BLANK ? altT * 10 : 0) + display.characters.LEFT_5;

      if (alt > 15) break;

      DMEs[waypointSelector - 1].altitude = alt;

      break;
    }
    case INSERT_MODE::DME_FREQ: {
      if (dataSelector != DATA_SELECTOR::WPT && dmeMode != DME_MODE::DME_FREQ) return;
      if (waypointSelector == 0) break;

      uint16_t freq = display.characters.RIGHT_2 * 10000 + display.characters.RIGHT_3 * 1000 +
        display.characters.RIGHT_4 * 100 + display.characters.RIGHT_5 * 10;

      uint8_t freqO = display.characters.RIGHT_6;
      if (freqO >= 4 && freqO < 9) {
        freq += 5;
      }

      if (freq < 10800 || freq > 13595) break;

      DMEs[waypointSelector - 1].frequency = freq;

      break;
    }
    case INSERT_MODE::PERFORMANCE_INDEX: {
      if (dataSelector != DATA_SELECTOR::DSRTKSTS) return;

      // Erradication
      if (display.characters.RIGHT_6 == 1) {
        activePerformanceIndex = 5;
        currentINSPosition = initialINSPosition;
        accuracyIndex = 0;
        updateSimPosDelta();
      }
      // Aided
      else if (display.characters.RIGHT_6 == 4) {
        activePerformanceIndex = 4;
      }
      // Unaided
      else {
        activePerformanceIndex = 5;
      }

      break;
    }
    case INSERT_MODE::WPT_CHG_FROM:
    case INSERT_MODE::WPT_CHG_TO: {
      // TODO: DME designation
      
      currentLegStart = display.characters.FROM;
      currentLegEnd = display.characters.TO;

      waypoints[0] = currentINSPosition;
      timeInLeg = 0;

      indicators.indicator.WAYPOINT_CHANGE = indicators.indicator.ALERT  = false;
      break;
    }
  }

  if (currentINSPosition.isValid()) {
    indicators.indicator.INSERT = false;
  }

  insertMode = INSERT_MODE::INV;
}

void INS::handleTestButtonState(const bool _state) noexcept {
  if (actionMalfunctionCodes.value != 0) {
    // Pressed
    if (_state) {
      if (mafunctionCodeDisplayed) {
        // NOTE: KEEP IN SYNC WITH DISPLAY FUNCTION
        if (displayActionMalfunctionCodeIndex == 5) {
          actionMalfunctionCodes.codes.A04_41 = false;
          config.setLastINSPosition(currentINSPosition);

          if (actionMalfunctionCodes.value == 0) {
            indicators.indicator.WARN = false;
          }
        }
        if (displayActionMalfunctionCodeIndex == 3) {
          actionMalfunctionCodes.codes.A02_49 = false;

          if (actionMalfunctionCodes.value == 0) {
            indicators.indicator.WARN = false;
          }
        }

        uint8_t last = displayActionMalfunctionCodeIndex;
        advanceActionMalfunctionIndex();

        if (last < 5 && displayActionMalfunctionCodeIndex >= 5 ||
            last >= 5 && displayActionMalfunctionCodeIndex < 5 ||
            last == displayActionMalfunctionCodeIndex) {
          mafunctionCodeDisplayed = false;
        }
      }
      else {
        mafunctionCodeDisplayed = true;
      }
    }
  }
  else {
    if (_state) {
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

void INS::handleDMEModeEntry(const uint8_t value) noexcept {
  if (dataSelector != DATA_SELECTOR::WPT && dataSelector != DATA_SELECTOR::DISTIME) return;

  if (value == 'L') {
    dmeMode = DME_MODE::DME_LL;
  }
  else if (value == 'F') {
    dmeMode = DME_MODE::DME_FREQ;
  }
}

void INS::handleClear() noexcept {
  // Only clear INSERT if PPos exists
  if (currentINSPosition.isValid()) {
    indicators.indicator.INSERT = false;
  }
  // Reset displayed
  if (insertMode == INSERT_MODE::PRE_POS_LON || insertMode == INSERT_MODE::POS_LON) {
    if (currentINSPosition.isValid()) {
      displayPosition = currentINSPosition;
    }
    else {
      displayPosition = config.getLastINSPosisiton();
    }
  }

  indicators.indicator.WAYPOINT_CHANGE = false;
  insertMode = INSERT_MODE::INV;
}

void INS::handleWaypointChange() noexcept {
  // Can't in OFF or ATT
  if (state == INS_STATE::OFF || state == INS_STATE::ATT) return;

  indicators.indicator.WAYPOINT_CHANGE = indicators.indicator.INSERT = true;
  insertMode = INSERT_MODE::WPT_CHG_FROM;
}

void INS::handleHoldButton() noexcept {
  // Can't in OFF or ATT
  if (state == INS_STATE::OFF || state == INS_STATE::ATT) return;

  if (inHoldMode) {
    if (holdRequiresForce) {
      currentError = { 0, 0 };
      timeInNAV = 0;
      currentINSPosition = displayPosition;
      updateSimPosDelta();
    }

    holdINSPosition = holdPosition = { 999, 999 };
    inHoldMode = holdRequiresForce = false;
    indicators.indicator.INSERT = false;
    insertMode = INSERT_MODE::INV;
  }
  else {
    holdINSPosition = initialINSPosition;
    holdPosition = currentINSPosition;
    inHoldMode = true;
  }

  indicators.indicator.HOLD = inHoldMode;
}

void INS::handleAutoMan() noexcept {
  autoMode = !autoMode;
}

void INS::handleInstantAlign() noexcept {
  if (modeSelector != MODE_SELECTOR::ALIGN) return;

  alignSubmode = ALIGN_SUBMODE::MODE_0;
  indicators.indicator.READY_NAV = true;
  initialTimeInNAV =  timeInNAV = timeInMode = 0;
}
