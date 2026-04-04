#include "ins/ins.h"

namespace libciva {

#pragma region Static Helpers

static void formatPos(DISPLAY &display, POSITION pos) noexcept {
  if (pos.latitude == 999) {
    display.characters.N = display.characters.S = false;
    pos.latitude = 0;
  } else if (pos.latitude < 0) {
    display.characters.N = false;
    display.characters.S = true;
    pos.latitude = -1 * pos.latitude;
  } else {
    display.characters.N = true;
    display.characters.S = false;
  }

  if (pos.longitude == 999) {
    display.characters.E = display.characters.W = false;
    pos.longitude = 0;
  } else if (pos.longitude < 0) {
    display.characters.E = false;
    display.characters.W = true;
    pos.longitude = -1 * pos.longitude;
  } else {
    display.characters.E = true;
    display.characters.W = false;
  }

  double degrees = std::trunc(pos.latitude);
  double minutes = std::round((pos.latitude - (uint8_t)pos.latitude) * 600);
  if (minutes >= 600) {
    minutes -= 600;
    degrees += 1;
  }

  display.characters.LEFT_1 = (uint8_t)degrees / 10;
  display.characters.LEFT_2 = (uint8_t)degrees - display.characters.LEFT_1 * 10;
  display.characters.LEFT_3 = (uint16_t)minutes / 100;
  display.characters.LEFT_4 = (uint16_t)minutes / 10 - display.characters.LEFT_3 * 10;
  display.characters.LEFT_5 = (uint16_t)minutes - display.characters.LEFT_3 * 100 - display.characters.LEFT_4 * 10;

  degrees = std::trunc(pos.longitude);
  minutes = std::round((pos.longitude - (uint16_t)pos.longitude) * 600);
  if (minutes >= 600) {
    minutes -= 600;
    degrees += 1;
  }
  display.characters.RIGHT_1 = (uint16_t)degrees / 100;
  display.characters.RIGHT_2 = (uint16_t)degrees / 10 - display.characters.RIGHT_1 * 10;
  display.characters.RIGHT_3 = (uint16_t)degrees - display.characters.RIGHT_1 * 100 - display.characters.RIGHT_2 * 10;
  display.characters.RIGHT_4 = (uint16_t)minutes / 100;
  display.characters.RIGHT_5 = (uint16_t)minutes / 10 - display.characters.RIGHT_4 * 10;
  display.characters.RIGHT_6 = (uint16_t)minutes - display.characters.RIGHT_4 * 100 - display.characters.RIGHT_5 * 10;
}

static void formatQuad(DISPLAY &display, const double value, const bool left, const bool hasDirection, const uint8_t dir,
                       const bool decimal = false) {
  uint8_t valueH = (uint8_t)((uint16_t)value / 1000);
  uint8_t valueT = (uint8_t)((uint16_t)(value - valueH * 1000) / 100);
  uint8_t valueO = (uint8_t)((uint16_t)(value - valueH * 1000 - valueT * 100) / 10);
  uint8_t valueD = (uint8_t)(value - valueH * 1000 - valueT * 100 - valueO * 10);

  if (left) {
    if (hasDirection) {
      display.characters.LEFT_1 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_2 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_3 = valueH > 0 || valueT > 0 || valueO > 0 || decimal ? valueO : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = valueD;
      display.characters.LEFT_5 = dir;
    } else {
      display.characters.LEFT_2 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_3 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = valueH > 0 || valueT > 0 || valueO > 0 || decimal ? valueO : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_5 = valueD;
    }
  } else {
    if (hasDirection) {
      display.characters.RIGHT_2 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_3 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_4 = valueH > 0 || valueT > 0 || valueO > 0 || decimal ? valueO : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = valueD;
      display.characters.RIGHT_6 = dir;
    } else {
      display.characters.RIGHT_3 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_4 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = valueH > 0 || valueT > 0 || valueO > 0 || decimal ? valueO : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_6 = valueD;
    }
  }
}

static void formatTri(DISPLAY &display, const double value, const bool left, const bool hasDirection, const uint8_t dir) {
  uint8_t valueH = (uint8_t)((uint16_t)value / 100);
  uint8_t valueT = (uint8_t)((uint16_t)(value - valueH * 100) / 10);
  uint8_t valueO = (uint8_t)((uint16_t)value - valueH * 100 - valueT * 10);

  if (left) {
    if (hasDirection) {
      display.characters.LEFT_2 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_3 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = valueO;
      display.characters.LEFT_5 = dir;
    } else {
      display.characters.LEFT_3 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_5 = valueO;
    }
  } else {
    if (hasDirection) {
      display.characters.RIGHT_3 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_4 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = valueO;
      display.characters.RIGHT_6 = dir;
    } else {
      display.characters.RIGHT_4 = valueH > 0 ? valueH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = valueH > 0 || valueT > 0 ? valueT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_6 = valueO;
    }
  }
}

#pragma endregion

void INS::formatActionMalfunctionCode(const bool showingMalf) noexcept {
  uint8_t action = 0;
  uint8_t malf = 0;

  switch (displayActionMalfunctionCodeIndex) {
    case 0:
      display.characters.RIGHT_2 = display.characters.RIGHT_3 = DISPLAY_CHAR_BLANK;
      return;
    case 1: {
      action = 2;
      malf = 31;
      break;
    }
    case 2: {
      action = 2;
      malf = 42;
      break;
    }
    case 3: {
      action = 2;
      malf = 49;
      break;
    }
    case 4: {
      action = 2;
      malf = 63;
      break;
    }
    case 5: {
      action = 4;
      malf = 41;
      break;
    }
    case 6: {
      action = 4;
      malf = 43;
      break;
    }
    case 7: {
      action = 4;
      malf = 57;
      break;
    }
  }

  if (showingMalf) {
    display.characters.RIGHT_2 = malf / 10;
    display.characters.RIGHT_3 = malf - (malf / 10) * 10;
  } else {
    display.characters.RIGHT_2 = 0;
    display.characters.RIGHT_3 = action;
  }
}

void INS::updateDisplay(const double dTime) noexcept {
  const POSITION pos = currentNavPosition(dTime);

  if (inTestMode) {
    uint64_t d;
    d = 0xFF88888888F88888;
    display = *(reinterpret_cast<DISPLAY *>(&d));
    uint64_t i;
    i = 0x00000000000019FF;
    indicators = *(reinterpret_cast<INDICATORS *>(&i));

    return;
  }
  if (insertMode == INSERT_MODE::INV || insertMode == INSERT_MODE::WPT_CHG_TO || insertMode == INSERT_MODE::WPT_CHG_FROM) {
    if (inHoldMode && !holdRequiresForce) {
      displayPosition = holdPosition;
    } else if (!holdRequiresForce) {
      displayPosition = pos;
    }
  }

  double gs = groundSpeed;

  // Main display
  switch (dataSelector) {
    case DATA_SELECTOR::TKGS: {
      uint16_t _track = (uint16_t)(std::round(track * 10));

      if (gs > MAX_GS_DISPLAY) {
        gs = MAX_GS_DISPLAY;
      } else {
        gs = std::round(gs);
      }

      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEG_1 = display.characters.RIGHT_DEC_1 =
          display.characters.RIGHT_DEC_2 = display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEG_2 =
              display.characters.N = display.characters.S = display.characters.E = display.characters.W = false;
      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_2 = true;
      display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 = display.characters.RIGHT_1 =
          display.characters.RIGHT_2 = display.characters.RIGHT_3 = display.characters.RIGHT_4 = display.characters.RIGHT_5 =
              DISPLAY_CHAR_BLANK;

      if (state <= INS_STATE::ALIGN &&
          (alignSubmode > ALIGN_SUBMODE::MODE_7 || (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode <= MAX_MODE_7))) {
        display.characters.LEFT_4 = display.characters.LEFT_5 = display.characters.RIGHT_6 = 0;
      } else {
        formatQuad(display, _track, true, false, 0, true);
        formatQuad(display, gs, false, false, 0);
      }

      break;
    }
    case DATA_SELECTOR::HDGDA: {
      uint16_t heading = (uint16_t)(std::round(varManager.sim.planeHeadingDegreesTrue * 10));

      int16_t driftAngle = (int16_t)std::round(((uint16_t)absDeltaAngle(heading / 10.0, track) % 180));
      uint8_t driftAngleDir = DISPLAY_CHAR_RIGHT;
      if (driftAngle <= 0) {
        driftAngle *= -1;
        driftAngleDir = DISPLAY_CHAR_LEFT;
      }

      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEG_1 = display.characters.RIGHT_DEC_1 =
          display.characters.RIGHT_DEC_2 = display.characters.RIGHT_DEG_1 = display.characters.N = display.characters.S =
              display.characters.E = display.characters.W = false;
      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_2 = display.characters.RIGHT_DEG_2 = true;
      display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 = display.characters.RIGHT_1 =
          display.characters.RIGHT_2 = display.characters.RIGHT_3 = display.characters.RIGHT_4 = display.characters.RIGHT_5 =
              DISPLAY_CHAR_BLANK;

      if (state <= INS_STATE::ALIGN &&
          (alignSubmode > ALIGN_SUBMODE::MODE_7 || (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode <= MAX_MODE_7))) {
        display.characters.LEFT_4 = display.characters.LEFT_5 = display.characters.RIGHT_6 = 0;
      } else {
        formatQuad(display, heading, true, false, 0, true);
        formatTri(display, driftAngle, false, true, driftAngleDir);
      }

      break;
    }
    case DATA_SELECTOR::XTKTKE: {
      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEC_2 = display.characters.LEFT_DEG_1 =
          display.characters.LEFT_DEG_2 = display.characters.RIGHT_DEC_1 = display.characters.RIGHT_DEC_2 =
              display.characters.RIGHT_DEG_1 = display.characters.N = display.characters.S = display.characters.E =
                  display.characters.W = false;
      display.characters.LEFT_DEC_1 = display.characters.RIGHT_DEG_2 = true;
      display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.RIGHT_1 = display.characters.RIGHT_2 =
          display.characters.RIGHT_3 = display.characters.RIGHT_4 = DISPLAY_CHAR_BLANK;
      display.characters.LEFT_3 = display.characters.LEFT_4 = display.characters.RIGHT_5 = 0;
      display.characters.LEFT_5 = display.characters.RIGHT_6 = DISPLAY_CHAR_LEFT;

      if (state >= INS_STATE::ALIGN && pos.isValid() &&
          (alignSubmode < ALIGN_SUBMODE::MODE_7 || (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode >= MAX_MODE_7))) {
        int16_t xtk = 0;
        if (insertMode == INSERT_MODE::WPT_CHG_FROM || insertMode == INSERT_MODE::WPT_CHG_TO) {
          xtk = (int16_t)std::round(std::min(
              9999.0, pos.crossTrackDistance(waypoints[display.characters.FROM], waypoints[display.characters.TO]) * 10));
        } else {
          xtk = (int16_t)std::round(std::min(9999.0, crossTrackError * 10));
        }

        uint8_t xtkDir = DISPLAY_CHAR_RIGHT;
        if (xtk <= 0) {
          xtk *= -1;
          xtkDir = DISPLAY_CHAR_LEFT;
        }
        formatQuad(display, xtk, true, true, xtkDir, true);

        int16_t tke = (int16_t)trackAngleError;
        uint8_t tkeDir = DISPLAY_CHAR_LEFT;
        if (tke < 0) {
          tke *= -1;
          tkeDir = DISPLAY_CHAR_RIGHT;
        }
        formatTri(display, tke, false, true, tkeDir);
      }

      break;
    }
    case DATA_SELECTOR::POS: {
      if (insertMode == INSERT_MODE::POS_LAT || insertMode == INSERT_MODE::PRE_POS_LON || insertMode == INSERT_MODE::POS_LON)
        break;

      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_1 = display.characters.RIGHT_DEG_2 =
          display.characters.RIGHT_DEC_1 = false;
      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 = display.characters.RIGHT_DEG_1 =
          display.characters.RIGHT_DEC_2 = true;

      formatPos(display, displayPosition.isValid() ? displayPosition : config->getLastINSPosition());

      break;
    }
    case DATA_SELECTOR::WPT: {
      if (insertMode == INSERT_MODE::WPT_LAT || insertMode == INSERT_MODE::WPT_LON || insertMode == INSERT_MODE::DME_ALT ||
          insertMode == INSERT_MODE::DME_FREQ)
        break;

      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_1 = display.characters.RIGHT_DEG_2 =
          display.characters.RIGHT_DEC_1 = false;
      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 = display.characters.RIGHT_DEG_1 =
          display.characters.RIGHT_DEC_2 = true;

      if (inHoldMode) {
        formatPos(display, holdINSPosition);
      } else if (dmeMode == DME_MODE::DME_LL) {
        formatPos(display, DMEs[std::max(0, waypointSelector - 1)].position);
      } else if (dmeMode == DME_MODE::DME_FREQ) {
        display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 = display.characters.RIGHT_1 =
            DISPLAY_CHAR_BLANK;
        display.characters.N = display.characters.S = display.characters.E = display.characters.W = false;

        DME dme = DMEs[std::max(0, waypointSelector - 1)];

        uint8_t altT = dme.altitude / 10;
        uint8_t altO = dme.altitude - altT * 10;
        display.characters.LEFT_4 = altT > 0 ? altT : DISPLAY_CHAR_BLANK;
        display.characters.LEFT_5 = altO;

        uint8_t freqTTH = (uint8_t)(dme.frequency / 10000);
        uint8_t freqTH = (uint8_t)(dme.frequency / 1000 - freqTTH * 10);
        uint8_t freqH = (uint8_t)(dme.frequency / 100 - freqTTH * 100 - freqTH * 10);
        uint8_t freqT = (uint8_t)(dme.frequency / 10 - freqTTH * 1000 - freqTH * 100 - freqH * 10);
        uint8_t freqO = (uint8_t)(dme.frequency - freqTTH * 10000 - freqTH * 1000 - freqH * 100 - freqT * 10);
        display.characters.RIGHT_2 = freqTTH;
        display.characters.RIGHT_3 = freqTH;
        display.characters.RIGHT_4 = freqH;
        display.characters.RIGHT_5 = freqT;
        display.characters.RIGHT_6 = freqO;
      } else {
        formatPos(display, waypoints[waypointSelector]);
      }

      break;
    }
    case DATA_SELECTOR::DISTIME: {
      double dist = 0;
      int16_t time = -1;

      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEC_2 = display.characters.LEFT_DEG_1 =
          display.characters.LEFT_DEG_2 = display.characters.RIGHT_DEC_1 = display.characters.RIGHT_DEG_1 =
              display.characters.RIGHT_DEG_2 = display.characters.N = display.characters.S = display.characters.E =
                  display.characters.W = false;
      display.characters.RIGHT_DEC_2 = true;
      display.characters.LEFT_1 = display.characters.RIGHT_1 = display.characters.RIGHT_2 = DISPLAY_CHAR_BLANK;

      if (dmeMode != DME_MODE::INV) {
        if (pos.isValid()) {
          dist = pos.distanceTo(DMEs[std::max(0, waypointSelector - 1)].position);
        }
      } else if (insertMode == INSERT_MODE::WPT_CHG_FROM || insertMode == INSERT_MODE::WPT_CHG_TO) {
        dist = waypoints[display.characters.FROM].distanceTo(waypoints[display.characters.TO]);
        if (state >= INS_STATE::ALIGN &&
            (alignSubmode < ALIGN_SUBMODE::MODE_7 || (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode >= MAX_MODE_7))) {
          time = gs > MIN_GS_TIME ? (int16_t)std::round(std::min(9999.0, (dist / gs) * 600)) : 9999;
        } else {
          time = 9999;
        }
      } else {
        dist = pos.distanceTo(waypoints[currentLegEnd]);
        if (state >= INS_STATE::ALIGN &&
            (alignSubmode < ALIGN_SUBMODE::MODE_7 || (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode >= MAX_MODE_7))) {
          time = gs > MIN_GS_TIME ? (int16_t)std::round(std::min(9999.0, (dist / gs) * 600)) : 9999;
        } else {
          time = 9999;
        }
      }

      formatQuad(display, (uint16_t)std::round(std::min(9999.0, dist)), true, false, 0);
      if (time >= 0) {
        formatQuad(display, time, false, false, 0, true);
      } else {
        display.characters.RIGHT_3 = display.characters.RIGHT_4 = display.characters.RIGHT_5 = display.characters.RIGHT_6 =
            DISPLAY_CHAR_BLANK;
        display.characters.RIGHT_DEC_2 = false;
      }

      break;
    }
    case DATA_SELECTOR::WIND: {
      double tas = varManager.sim.airspeedTrue;

      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEC_2 = display.characters.LEFT_DEG_1 =
          display.characters.RIGHT_DEC_1 = display.characters.RIGHT_DEC_2 = display.characters.RIGHT_DEG_1 =
              display.characters.RIGHT_DEG_2 = display.characters.N = display.characters.S = display.characters.E =
                  display.characters.W = false;
      display.characters.LEFT_DEG_2 = true;
      display.characters.LEFT_1 = display.characters.LEFT_2 = display.characters.LEFT_3 = display.characters.LEFT_4 =
          display.characters.RIGHT_1 = display.characters.RIGHT_2 = display.characters.RIGHT_3 = display.characters.RIGHT_4 =
              display.characters.RIGHT_5 = DISPLAY_CHAR_BLANK;

      if (tas <= 0 || gs < MIN_GS || tas < MIN_TAS_WIND || tas > MAX_TAS_WIND ||
          !(state >= INS_STATE::ALIGN &&
            (alignSubmode < ALIGN_SUBMODE::MODE_7 || (alignSubmode == ALIGN_SUBMODE::MODE_7 && timeInMode >= MAX_MODE_7)))) {
        display.characters.LEFT_5 = display.characters.RIGHT_6 = 0;
      } else {
        double dir = std::round(varManager.sim.ambientWindDirection);
        double speed = std::min(std::round(varManager.sim.ambientWindVelocity), 606.0);

        formatTri(display, dir, true, false, 0);
        formatTri(display, speed, false, false, 0);
      }

      break;
    }
    case DATA_SELECTOR::DSRTKSTS: {
      display.characters.LEFT_DEC_1 = display.characters.LEFT_DEC_2 = display.characters.LEFT_DEG_1 =
          display.characters.LEFT_DEG_2 = display.characters.RIGHT_DEC_1 = display.characters.RIGHT_DEC_2 =
              display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEG_2 = display.characters.N = display.characters.S =
                  display.characters.E = display.characters.W = false;
      display.characters.LEFT_3 = display.characters.RIGHT_4 = DISPLAY_CHAR_BLANK;

      if (pos.isValid()) {
        display.characters.LEFT_1 = DISPLAY_CHAR_BLANK;
        display.characters.LEFT_2 = DISPLAY_CHAR_BLANK;
        display.characters.LEFT_DEG_2 = true;

        uint16_t crs = 0;

        if (insertMode == INSERT_MODE::WPT_CHG_FROM || insertMode == INSERT_MODE::WPT_CHG_TO) {
          crs = (uint16_t)std::round(waypoints[display.characters.FROM].bearingTo(waypoints[display.characters.TO]));
        } else {
          crs = (uint16_t)std::round(desiredTrack);
        }

        formatTri(display, crs, true, false, 0);
      } else {
        display.characters.LEFT_1 = PROG_NUM[0];
        display.characters.LEFT_2 = PROG_NUM[1];
        display.characters.LEFT_4 = PROG_NUM[2];
        display.characters.LEFT_5 = PROG_NUM[3];
      }

      if (state == INS_STATE::NAV) {
        display.characters.RIGHT_1 = 1;
        display.characters.RIGHT_5 = std::min(accuracyIndex, (uint8_t)9);
      } else {
        display.characters.RIGHT_1 = 0;
        display.characters.RIGHT_5 = (uint8_t)alignSubmode;
      }

      formatActionMalfunctionCode(malfunctionCodeDisplayed);

      if (insertMode != INSERT_MODE::PERFORMANCE_INDEX) {
        display.characters.RIGHT_6 = (uint8_t)activePerformanceIndex;
      }

      break;
    }
  }
  // TO/FROM
  switch (dataSelector) {
    case DATA_SELECTOR::WPT: {
      if (dmeMode != DME_MODE::INV) {
        display.characters.FROM = DISPLAY_CHAR_BLANK;
        if (insertMode == INSERT_MODE::INV) display.characters.TO = activeDME;
        indicators.indicator.TO_BLINK = insertMode != INSERT_MODE::WPT_CHG_TO ? true : false;
        indicators.indicator.FROM_BLINK = false;
      } else if (insertMode != INSERT_MODE::WPT_CHG_FROM && insertMode != INSERT_MODE::WPT_CHG_TO) {
        display.characters.FROM = waypointSelector;
        display.characters.TO = DISPLAY_CHAR_BLANK;
        indicators.indicator.TO_BLINK = indicators.indicator.FROM_BLINK = false;
      }
      break;
    }
    case DATA_SELECTOR::DISTIME: {
      if (dmeMode != DME_MODE::INV) {
        display.characters.FROM = DISPLAY_CHAR_BLANK;
        if (insertMode == INSERT_MODE::INV) display.characters.TO = activeDME;
        indicators.indicator.TO_BLINK = insertMode != INSERT_MODE::WPT_CHG_TO ? true : false;
        ;
        indicators.indicator.FROM_BLINK = false;
      } else if (insertMode != INSERT_MODE::WPT_CHG_FROM && insertMode != INSERT_MODE::WPT_CHG_TO) {
        display.characters.FROM = currentLegStart;
        display.characters.TO = currentLegEnd;
        indicators.indicator.TO_BLINK = indicators.indicator.FROM_BLINK = false;
      }
      break;
    }
    default: {
      if (insertMode != INSERT_MODE::WPT_CHG_FROM && insertMode != INSERT_MODE::WPT_CHG_TO) {
        display.characters.FROM = currentLegStart;
        display.characters.TO = currentLegEnd;
        indicators.indicator.FROM_BLINK = indicators.indicator.TO_BLINK = false;
      }

      break;
    }
  }
}

void INS::alertLamp(const double dTime) noexcept {
  if (groundSpeed >= ALERT_MIN_GS) {
    const POSITION pos = currentNavPosition(dTime);

    double remDist = pos.distanceTo(waypoints[currentLegEnd]);

    double remTime = (remDist / groundSpeed) * 3600;

    if (!waypoints[currentLegEnd].inFront(pos, track)) {
      // We passed
      if (autoModePassed || !autoMode) {
        // Either in manual mode or flight time is < 25.6 in auto mode
        if (flashTimer < 1) {
          indicators.indicator.ALERT = true;
          flashTimer += dTime;
        } else if (flashTimer < 2) {
          indicators.indicator.ALERT = false;
          flashTimer += dTime;
        } else {
          flashTimer = 0;
        }
      } else {
        // Auto mode, this should hit when switching from MAN to AUTO
        indicators.indicator.ALERT = false;
      }
    } else if (remTime <= LEG_TIME_ALERT) {
      // 2min alert
      indicators.indicator.ALERT = true;
    }
  }
}

} // namespace libciva