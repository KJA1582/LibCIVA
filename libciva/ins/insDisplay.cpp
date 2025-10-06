#include "ins.h"

static void formatPos(DISPLAY &display, POSITION &pos) noexcept {
  if (pos.latitude == 999) {
    display.characters.N = display.characters.S = false;
    pos.latitude = 0;
  }
  else if (pos.latitude < 0) {
    display.characters.N = false;
    display.characters.S = true;
    pos.latitude = -1 * pos.latitude;
  }
  else {
    display.characters.N = true;
    display.characters.S = false;
  }

  if (pos.longitude == 999) {
    display.characters.E = display.characters.W = false;
    pos.longitude = 0;
  }
  else if (pos.longitude < 0) {
    display.characters.E = false;
    display.characters.W = true;
    pos.longitude = -1 * pos.longitude;
  }
  else {
    display.characters.E = true;
    display.characters.W = false;
  }

  display.characters.LEFT_1 = (uint8_t)pos.latitude / 10;
  display.characters.LEFT_2 = (uint8_t)pos.latitude - display.characters.LEFT_1 * 10;
  double minutes = std::round((pos.latitude - (uint8_t)pos.latitude) * 600);
  display.characters.LEFT_3 = (uint16_t)minutes / 100;
  display.characters.LEFT_4 = (uint16_t)minutes / 10 - display.characters.LEFT_3 * 10;
  display.characters.LEFT_5 = (uint16_t)minutes - display.characters.LEFT_3 * 100 -
    display.characters.LEFT_4 * 10;

  display.characters.RIGHT_1 = (uint16_t)pos.longitude / 100;
  display.characters.RIGHT_2 = (uint16_t)pos.longitude / 10 - display.characters.RIGHT_1 * 10;
  display.characters.RIGHT_3 = (uint16_t)pos.longitude - display.characters.RIGHT_1 * 100 -
    display.characters.RIGHT_2 * 10;
  minutes = std::round((pos.longitude - (uint16_t)pos.longitude) * 600);
  display.characters.RIGHT_4 = (uint16_t)minutes / 100;
  display.characters.RIGHT_5 = (uint16_t)minutes / 10 - display.characters.RIGHT_4 * 10;
  display.characters.RIGHT_6 = (uint16_t)minutes - display.characters.RIGHT_4 * 100 -
    display.characters.RIGHT_5 * 10;
}

static void formatQuad(DISPLAY &display, const double value, const bool left) {
  uint8_t valueH = (uint16_t)value / 1000;
  uint8_t valueT = (uint16_t)(value - valueH * 1000) / 100;
  uint8_t valueO = (uint16_t)(value - valueH * 1000 - valueT * 100) / 10;
  uint8_t valueD = (uint8_t)(value - valueH * 1000 - valueT * 100 - valueO * 10);

  if (left) {
    display.characters.LEFT_2 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
    display.characters.LEFT_3 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
    display.characters.LEFT_4 = valueH > 0 || valueT > 0 || valueO > 0 ? valueO : DISPLAY_CHAR_BLANK;
    display.characters.LEFT_5 = valueD;
  }
  else {
    display.characters.RIGHT_3 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
    display.characters.RIGHT_4 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
    display.characters.RIGHT_5 = valueH > 0 || valueT > 0 || valueO > 0 ? valueO : DISPLAY_CHAR_BLANK;
    display.characters.RIGHT_6 = valueD;
  }
}

static void formatTri(DISPLAY &display, const double value, const bool left,
                      const bool hasDirection, const uint8_t dir) {
  uint8_t valueH = (uint16_t)value / 100;
  uint8_t valueT = (uint16_t)(value - valueH * 100) / 10;
  uint8_t valueO = (uint16_t)value - valueH * 100 - valueT * 10;

  if (left) {
    if (hasDirection) {
      display.characters.LEFT_2 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_3 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = valueO;
      display.characters.LEFT_5 = dir;
    }
    else {
      display.characters.LEFT_3 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_5 = valueO;
    }
  }
  else {
    if (hasDirection) {
      display.characters.RIGHT_3 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_4 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = valueO;
      display.characters.RIGHT_6 = dir;
    }
    else {
      display.characters.RIGHT_4 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_6 = valueO;
    }
  }
}

static void formatActionMalfunctionCode(DISPLAY &display, const ACTION_MALFUNCTION_CODE malf,
                                        const bool showingMalf) {
  if (malf == ACTION_MALFUNCTION_CODE::INV) {
    display.characters.RIGHT_2 = display.characters.RIGHT_3 = DISPLAY_CHAR_BLANK;
    return;
  }

  if (showingMalf) {
    switch (malf) {
      case ACTION_MALFUNCTION_CODE::INV: {
        break;
      }
      case ACTION_MALFUNCTION_CODE::A02_31: {
        display.characters.RIGHT_2 = 3;
        display.characters.RIGHT_3 = 1;

        break;
      }
      case ACTION_MALFUNCTION_CODE::A02_42: {
        display.characters.RIGHT_2 = 4;
        display.characters.RIGHT_3 = 2;

        break;
      }
      case ACTION_MALFUNCTION_CODE::A02_49: {
        display.characters.RIGHT_2 = 4;
        display.characters.RIGHT_3 = 9;

        break;
      }
      case ACTION_MALFUNCTION_CODE::A02_63: {
        display.characters.RIGHT_2 = 6;
        display.characters.RIGHT_3 = 3;

        break;
      }
      case ACTION_MALFUNCTION_CODE::A04_41: {
        display.characters.RIGHT_2 = 4;
        display.characters.RIGHT_3 = 1;

        break;
      }
      case ACTION_MALFUNCTION_CODE::A04_43: {
        display.characters.RIGHT_2 = 4;
        display.characters.RIGHT_3 = 3;

        break;
      }
      case ACTION_MALFUNCTION_CODE::A04_57: {
        display.characters.RIGHT_2 = 5;
        display.characters.RIGHT_3 = 7;

        break;
      }
    }
  }
  else {
    switch (malf) {
      case ACTION_MALFUNCTION_CODE::INV: {
        break;
      }
      case ACTION_MALFUNCTION_CODE::A02_31:
      case ACTION_MALFUNCTION_CODE::A02_42:
      case ACTION_MALFUNCTION_CODE::A02_49:
      case ACTION_MALFUNCTION_CODE::A02_63: {
        display.characters.RIGHT_2 = 0;
        display.characters.RIGHT_3 = 2;

        break;
      }
      case ACTION_MALFUNCTION_CODE::A04_41:
      case ACTION_MALFUNCTION_CODE::A04_43:
      case ACTION_MALFUNCTION_CODE::A04_57: {
        display.characters.RIGHT_2 = 0;
        display.characters.RIGHT_3 = 4;

        break;
      }
    }
  }
}

void INS::updateDisplay() noexcept {
  if (inTestMode) {
    uint64_t d;
    d = 0xFF88888888F88888;
    display = *(reinterpret_cast<DISPLAY *>(&d));
    uint64_t i;
    i = 0x00000000000001FF;
    indicators = *(reinterpret_cast<INDICATORS *>(&i));

    return;
  }

  double gs;
  bool gsValid = varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs);
  double trueHeading;
  bool trueHeadingValid = varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading);

  // Main display
  switch (dataSelector) {
    case DATA_SELECTOR::TKGS: {
      uint16_t _track = 0;

      if (!gsValid) {
        gs = 0;

        if (trueHeadingValid || gs < MIN_GS) {
          _track = (uint16_t)(std::round(trueHeading * 10));
        }
      }
      else {
        _track = (uint16_t)(std::round(track * 10));
      }
      if (gs > MAX_GS_DISPLAY) {
        gs = MAX_GS_DISPLAY;
      }
      else {
        gs = std::round(gs);
      }

      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEG_1 =
        display.characters.RIGHT_DEC_1 = display.characters.RIGHT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEG_2 = display.characters.N =
        display.characters.S = display.characters.E = display.characters.W = false;
      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_2 = true;
      display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
        display.characters.LEFT_4 = display.characters.RIGHT_1 = display.characters.RIGHT_2 =
        display.characters.RIGHT_3 = display.characters.RIGHT_4 = display.characters.RIGHT_5 =
        DISPLAY_CHAR_BLANK;

      if (state <= INS_STATE::ALIGN && (alignSubmode > ALIGN_SUBMODE::MODE_7 ||
                                        (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode <= MAX_MODE_7))) {
        display.characters.LEFT_5 = display.characters.RIGHT_6 = 0;
      }
      else {
        formatQuad(display, _track, true);
        formatQuad(display, gs, false);
      }

      break;
    }
    case DATA_SELECTOR::HDGDA: {
      uint16_t heading = 0;

      if (!trueHeadingValid) {
        heading = 0;
      }
      else {
        heading = (uint16_t)(std::round(trueHeading * 10));
      }

      int16_t driftAngle = (int16_t)std::round(((heading - (uint16_t)(std::round(track * 10))) % 1800) / 10.0);
      uint8_t driftAngleDir = DISPLAY_CHAR_LEFT;
      if (driftAngle < 0) {
        driftAngle *= -1;
        driftAngleDir = DISPLAY_CHAR_RIGHT;
      }

      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEG_1 =
        display.characters.RIGHT_DEC_1 = display.characters.RIGHT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.N = display.characters.S =
        display.characters.E = display.characters.W = false;
      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_2 =
        display.characters.RIGHT_DEG_2 = true;
      display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
        display.characters.LEFT_4 = display.characters.RIGHT_1 = display.characters.RIGHT_2 =
        display.characters.RIGHT_3 = display.characters.RIGHT_4 = display.characters.RIGHT_5 =
        DISPLAY_CHAR_BLANK;

      if (state <= INS_STATE::ALIGN && (alignSubmode > ALIGN_SUBMODE::MODE_7 ||
                                        (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode <= MAX_MODE_7))) {
        display.characters.LEFT_5 = display.characters.RIGHT_6 = 0;
      }
      else {
        formatQuad(display, heading, true);
        formatTri(display, driftAngle, false, true, driftAngleDir);
      }

      break;
    }
    case DATA_SELECTOR::XTKTKE: {
      // TODO: XTK/TKE
      clearDisplay();
      break;
    }
    case DATA_SELECTOR::POS: {
      if (insertMode != INSERT_MODE::INV) break;

      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_1 =
        display.characters.RIGHT_DEG_2 = display.characters.RIGHT_DEC_1 = false;
      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEC_2 = true;

      formatPos(display, displayPosition);

      break;
    }
    case DATA_SELECTOR::WPT: {
      if (insertMode != INSERT_MODE::INV) break;

      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_1 =
        display.characters.RIGHT_DEG_2 = display.characters.RIGHT_DEC_1 = false;
      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEC_2 = true;

      if (dmeMode == DME_MODE::DME_LL) {
        formatPos(display, DMEs[std::max(0, waypointSelector - 1)].position);
      }
      else if (dmeMode == DME_MODE::DME_FREQ) {
        display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
          display.characters.RIGHT_1 = DISPLAY_CHAR_BLANK;
        display.characters.N = display.characters.S = display.characters.E = display.characters.W = false;

        DME dme = DMEs[std::max(0, waypointSelector - 1)];

        uint8_t altT = dme.altitude / 10;
        uint8_t altO = dme.altitude - altT * 10;
        display.characters.LEFT_4 = altT > 0 ? altT : DISPLAY_CHAR_BLANK;
        display.characters.LEFT_5 = altO;

        uint8_t freqTTH = dme.frequency / 10000;
        uint8_t freqTH = dme.frequency / 1000 - freqTTH * 10;
        uint8_t freqH = dme.frequency / 100 - freqTTH * 100 - freqTH * 10;
        uint8_t freqT = dme.frequency / 10 - freqTTH * 1000 - freqTH * 100 - freqH * 10;
        uint8_t freqO = dme.frequency - freqTTH * 10000 - freqTH * 1000 - freqH * 100 - freqT * 10;
        display.characters.RIGHT_2 = freqTTH;
        display.characters.RIGHT_3 = freqTH;
        display.characters.RIGHT_4 = freqH;
        display.characters.RIGHT_5 = freqT;
        display.characters.RIGHT_6 = freqO;
      }
      else {
        formatPos(display, waypoints[waypointSelector]);
      }

      break;
    }
    case DATA_SELECTOR::DISTIME: {
      // TODO: DIST/TIME
      clearDisplay();
      break;
    }
    case DATA_SELECTOR::WIND: {
      double tas = 0;
      bool valid = varManager.getVar(SIM_VAR_AIRSPEED_TRUE, tas);

      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEC_2 =
        display.characters.LEFT_DEG_1 = display.characters.RIGHT_DEC_1 =
        display.characters.RIGHT_DEC_2 = display.characters.RIGHT_DEG_1 =
        display.characters.RIGHT_DEG_2 = display.characters.N = display.characters.S =
        display.characters.E = display.characters.W = false;
      display.characters.LEFT_DEG_2 = true;
      display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 =
        display.characters.LEFT_4 = display.characters.RIGHT_1 = display.characters.RIGHT_2 =
        display.characters.RIGHT_3 = display.characters.RIGHT_4 = display.characters.RIGHT_5 =
        DISPLAY_CHAR_BLANK;

      if (!valid || gs < MIN_GS || tas < MIN_TAS_WIND || tas > MAX_TAS_WIND) {
        display.characters.LEFT_5 = display.characters.RIGHT_6 = 0;
      }
      else {
        double dir = 0;
        double speed = 0;
        if (varManager.getVar(SIM_VAR_AMBIENT_WIND_DIRECTION, dir) &&
            varManager.getVar(SIM_VAR_AMBIENT_WIND_VELOCITY, speed)) {
          dir = std::round(dir);
          speed = std::min(std::round(speed), 606.0);

          formatTri(display, dir, true, false, 0);
          formatTri(display, speed, false, false, 0);
        }
      }

      break;
    }
    case DATA_SELECTOR::DSRTKSTS: {
      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEC_2 =
        display.characters.LEFT_DEG_1 = display.characters.LEFT_DEG_2 =
        display.characters.RIGHT_DEC_1 = display.characters.RIGHT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEG_2 = display.characters.N =
        display.characters.S = display.characters.E = display.characters.W = false;
      display.characters.LEFT_3 = display.characters.RIGHT_4 = DISPLAY_CHAR_BLANK;

      if (currentINSPosition.isValid()) {
        display.characters.LEFT_1 = DISPLAY_CHAR_BLANK;
        display.characters.LEFT_2 = DISPLAY_CHAR_BLANK;
        // TODO: Desired Track
        display.characters.LEFT_DEG_2 = true;
      }
      else {
        display.characters.LEFT_1 = PROG_NUM[0];
        display.characters.LEFT_2 = PROG_NUM[1];
        display.characters.LEFT_4 = PROG_NUM[2];
        display.characters.LEFT_5 = PROG_NUM[3];
      }

      if (state == INS_STATE::NAV) {
        display.characters.RIGHT_1 = 1;
        display.characters.RIGHT_5 = accuracyIndex;
      }
      else {
        display.characters.RIGHT_1 = 0;
        display.characters.RIGHT_5 = (uint8_t)alignSubmode;
      }

      formatActionMalfunctionCode(display, getCurrentActionMalfunctionCode(), mafunctionCodeDisplayed);

      display.characters.RIGHT_6 =
        displayPerformanceIndex > 0 ? displayPerformanceIndex : activePerformanceIndex;

      break;
    }
  }
  // TO/FROM
  switch (dataSelector) {
    case DATA_SELECTOR::WPT: {
      if (dmeMode != DME_MODE::INV) {
        display.characters.FROM = DISPLAY_CHAR_BLANK;
        display.characters.TO = activeDME;
        indicators.indicator.TO_BLINK = true;
        indicators.indicator.FROM_BLINK = false;
      }
      else {
        display.characters.FROM = waypointSelector;
        display.characters.TO = DISPLAY_CHAR_BLANK;
        indicators.indicator.TO_BLINK = false;
        indicators.indicator.FROM_BLINK = false;
      }
      break;
    }
    case DATA_SELECTOR::DISTIME:
    {
      if (dmeMode != DME_MODE::INV) {
        display.characters.FROM = DISPLAY_CHAR_BLANK;
        display.characters.TO = activeDME;
        indicators.indicator.TO_BLINK = true;
        indicators.indicator.FROM_BLINK = false;
      }
      else {
        display.characters.FROM = currentLegStart;
        display.characters.TO = currentLegEnd;
        indicators.indicator.TO_BLINK = indicators.indicator.FROM_BLINK = false;
      }
      break;
    }
    default: {
      display.characters.FROM = currentLegStart;
      display.characters.TO = currentLegEnd;
      indicators.indicator.FROM_BLINK = indicators.indicator.TO_BLINK = false;

      break;
    }
  }

}