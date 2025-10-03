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

  display.characters.LEFT_1 = (char)lat / 10;
  display.characters.LEFT_2 = (char)lat - display.characters.LEFT_1 * 10;
  double minutes = round((lat - (char)lat) * 600);
  display.characters.LEFT_3 = (char)minutes / 100;
  display.characters.LEFT_4 = (char)minutes / 10 - display.characters.LEFT_3 * 10;
  display.characters.LEFT_5 = (char)minutes - display.characters.LEFT_3 * 100 -
    display.characters.LEFT_4 * 10;

  display.characters.RIGHT_1 = (short)lon / 100;
  display.characters.RIGHT_2 = (short)lon / 10 - display.characters.RIGHT_1 * 10;
  display.characters.RIGHT_3 = (short)lon - display.characters.RIGHT_1 * 100 -
    display.characters.RIGHT_2 * 10;
  minutes = round((lon - (short)lon) * 600);
  display.characters.RIGHT_4 = (char)minutes / 100;
  display.characters.RIGHT_5 = (char)minutes / 10 - display.characters.RIGHT_4 * 10;
  display.characters.RIGHT_6 = (char)minutes - display.characters.RIGHT_4 * 100 -
    display.characters.RIGHT_5 * 10;
}

void INS::display() const noexcept {
  DISPLAY display = getDisplay();
  DATA_SELECTOR_POS dataPos = getDataSelectorPos();
  INSERT_MODE insertMode = getInsertMode();
  WPT_SELECTOR_POS wptSelector = getWPTSelectorPos();

  double gs;
  bool gsValid = varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs);
  double trueHeading;
  bool trueHeadingValid = varManager.getVar(SIM_VAR_PLANE_HEADING_DEGREES_TRUE, trueHeading);

  switch (dataPos) {
    case DATA_SELECTOR_POS::TKGS: {
      short track = 0;

      if (!gsValid) {
        gs = 0;

        if (trueHeadingValid || gs < MIN_GS) {
          track = (short)(round(trueHeading * 10));
        }
      }
      else {
        track = (short)(round(getTrack() * 10));
      }
      if (gs > MAX_GS_DISPLAY) {
        gs = MAX_GS_DISPLAY;
      }
      else {
        gs = round(gs);
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

      char trackH = (short)track / 1000;
      char trackT = (short)(track - trackH * 1000) / 100;
      char trackO = (short)(track - trackH * 1000 - trackT * 100) / 10;
      char trackD = (char)(track - trackH * 1000 - trackT * 100 - trackO * 10);

      display.characters.LEFT_2 = trackH > 0 ? trackH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_3 = trackH > 0 || trackT > 0 ? trackT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = trackO;
      display.characters.LEFT_5 = trackD;

      char speedTH = (short)gs / 1000;
      char speedH = (short)(gs - speedTH * 1000) / 100;
      char speedT = (short)(gs - speedTH * 1000 - speedH * 100) / 10;
      char speedO = (short)gs - speedTH * 1000  - speedH * 100 - speedT * 10;

      display.characters.RIGHT_3 = speedTH > 0 ? speedTH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_4 = speedH > 0 || speedTH > 0 ? speedH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = speedH > 0 || speedT > 0 ? speedT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_6 = speedO;

      break;
    }
    case DATA_SELECTOR_POS::HDGDA: {
      short heading = 0;

      if (!trueHeadingValid) {
        heading = 0;
      }
      else {
        heading = (short)(round(trueHeading * 10));
      }

      short driftAngle = (short)round(((heading - (short)(round(getTrack() * 10))) % 1800) / 10.0);
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

      char headingH = (short)heading / 1000;
      char headingT = (short)(heading - headingH * 1000) / 100;
      char headingO = (short)(heading - headingH * 1000 - headingT * 100) / 10;
      char headingD = (char)(heading - headingH * 1000 - headingT * 100 - headingO * 10);

      display.characters.LEFT_2 = headingH > 0 ? headingH : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_3 = headingH > 0 || headingT > 0 ? headingT : DISPLAY_CHAR_BLANK;
      display.characters.LEFT_4 = headingO;
      display.characters.LEFT_5 = headingD;

      char driftAngleH = (short)driftAngle / 100;
      char drigtAngleT = (short)(driftAngle - driftAngleH * 100) / 10;
      char drigtAngleO = (short)driftAngle - driftAngleH * 100 - drigtAngleT * 10;

      display.characters.RIGHT_3 = driftAngleH > 0 ? driftAngleH : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_4 = driftAngleH > 0 || drigtAngleT > 0 ? drigtAngleT : DISPLAY_CHAR_BLANK;
      display.characters.RIGHT_5 = drigtAngleO;
      display.characters.RIGHT_6 = driftAngleDir;

      break;
    }
    case DATA_SELECTOR_POS::XTKTKE: {
      break;
    }
    case DATA_SELECTOR_POS::POS: {
      if (insertMode != INSERT_MODE::INV) break;

      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_1 =
        display.characters.RIGHT_DEG_2 = display.characters.RIGHT_DEC_1 = false;
      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEC_2 = true;

      double lat = getDisplayPosLat();
      double lon = getDisplayPosLon();

      formatPos(display, lat, lon);

      break;
    }
    case DATA_SELECTOR_POS::WPT:
    {
      if (insertMode != INSERT_MODE::INV) break;

      display.characters.LEFT_DEG_2 = display.characters.LEFT_DEC_1 =
        display.characters.RIGHT_DEG_2 = display.characters.RIGHT_DEC_1 = false;
      display.characters.LEFT_DEG_1 = display.characters.LEFT_DEC_2 =
        display.characters.RIGHT_DEG_1 = display.characters.RIGHT_DEC_2 = true;

      double lat = getWPTPosLat(wptSelector);
      double lon = getWPTPosLon(wptSelector);

      formatPos(display, lat, lon);

      break;
    }
    case DATA_SELECTOR_POS::DISTIME: {
      break;
    }
    case DATA_SELECTOR_POS::WIND: {
      double tas;
      double gs;
      bool valid = varManager.getVar(SIM_VAR_AIRSPEED_TRUE, tas) &&
        varManager.getVar(SIM_VAR_GROUND_VELOCITY, gs);

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
        double dir;
        double speed;
        if (varManager.getVar(SIM_VAR_AMBIENT_WIND_DIRECTION, dir) &&
            varManager.getVar(SIM_VAR_AMBIENT_WIND_VELOCITY, speed)) {
          dir = round(dir);
          speed = fmin(round(speed), 606.0);

          char dirH = (short)dir / 100;
          char dirT = (short)(dir - dirH * 100) / 10;
          char dirO = (short)dir - dirH * 100 - dirT * 10;

          display.characters.LEFT_3 = dirH > 0 ? dirH : DISPLAY_CHAR_BLANK;
          display.characters.LEFT_4 = dirH > 0 || dirT > 0 ? dirT : DISPLAY_CHAR_BLANK;
          display.characters.LEFT_5 = dirO;

          char speedH = (short)speed / 100;
          char speedT = (short)(speed - speedH * 100) / 10;
          char speedO = (short)speed - speedH * 100 - speedT * 10;

          display.characters.RIGHT_4 = speedH > 0 ? speedH : DISPLAY_CHAR_BLANK;
          display.characters.RIGHT_5 = speedH > 0 || speedT > 0 ? speedT : DISPLAY_CHAR_BLANK;
          display.characters.RIGHT_6 = speedO;
        }
      }

      break;
    }
    case DATA_SELECTOR_POS::DSRTKSTS: {
      break;
    }
  }

  setDisplay(display);
}