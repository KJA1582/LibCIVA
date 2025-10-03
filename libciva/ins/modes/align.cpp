#include "../ins.h"

double INS::align() noexcept {
  double operatingTime = getOperatingTime();
  INS_STATE state = getINSState();
  MODE_SELECTOR_POS mode = getModeSelectorPos();
  INDICATORS indicators = getIndicators();
  ALIGN_SUBMODE submode = getAlignSubmode();
  double displayLat = getDisplayPosLat();
  double displayLon = getDisplayPosLon();
  double insLat = getINSPosLat();
  double insLon = getINSPosLon();

  if (mode == MODE_SELECTOR_POS::STBY) {
    // Downmode
    setINSState(INS_STATE::STBY);
    reset(false);

    setINSPosLat(999);
    setINSPosLon(999);

    return 0;
  }
  else if (mode == MODE_SELECTOR_POS::NAV) {
    // Upmode
    if (submode <= ALIGN_SUBMODE::MODE_5) {
      setINSState(INS_STATE::NAV);
      indicators.indicator.READY_NAV = false;
      setIndicators(indicators);
      // FIXME: To truly get the downside of AI5 NAV, save the OG AI on nav entry as a scalar
      // Meaning, at AI0, error increases less fast than at AI5. POS updating will reset scalar and AI to AI0 level
      setAccuracyIndex(ACCURACY_INDEX::AI_0);

      return 0;
    }
  }

  switch (submode) {
    case ALIGN_SUBMODE::INV: {
      // ERROR CASE
      break;
    }
    case ALIGN_SUBMODE::MODE_9: {
      if (getTemperature() >= config.getOperatingTempInC()) {
        setAlignSubmode(ALIGN_SUBMODE::MODE_8);
        operatingTime = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_8: {
      BATTERY_TEST batteryTestState = getBatteryTestState();

      if (batteryTestState == BATTERY_TEST::IDLE && (int)state != (int)mode) {
        setBatteryTestState(BATTERY_TEST::INHIBITED);
      }
      else if (batteryTestState == BATTERY_TEST::IDLE && operatingTime < MAX_BAT_TEST_TIME) {
        indicators.indicator.CDU_BAT = true;
        setIndicators(indicators);
        setBatteryTestState(BATTERY_TEST::RUNNING);
      }
      else if (batteryTestState == BATTERY_TEST::RUNNING && operatingTime >= MAX_BAT_TEST_TIME) {
        indicators.indicator.CDU_BAT = false;
        setIndicators(indicators);
        setBatteryTestState(BATTERY_TEST::COMPLETED);
      }

      // TODO: 04-43 for dual/triple INS
      double lastLat = config.getLastLat();
      double lastLon = config.getLastLon();
      if (lastLat == 999 && lastLon == 999) {
        config.setLastLat(displayLat);
        config.setLastLon(displayLon);
      }
      else if (isPosValid(insLat, insLon) &&
               distanceInNMI(insLat, insLon, lastLat, lastLon) > MAX_RAMP_DEV) {
        addActionMalfunctionCode(ACTION_MALFUNCTION_CODE::A04_41);
        indicators.indicator.WARN = true;
        setIndicators(indicators);
        // FIXME: In malf clear handler, set last pos
      }

      if (operatingTime >= MIN_MODE_8) {
        setAlignSubmode(ALIGN_SUBMODE::MODE_7);
        operatingTime = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_7: {
      if (operatingTime >= MAX_MODE_7 && isPosValid(insLat, insLat) &&
          !hasActionMalfunctionCode()) {
        setAlignSubmode(ALIGN_SUBMODE::MODE_6);
        operatingTime = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_6: {
      if (operatingTime >= MAX_MODE_6) {
        setAlignSubmode(ALIGN_SUBMODE::MODE_5);

        indicators.indicator.READY_NAV = true;

        operatingTime = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_5:
    case ALIGN_SUBMODE::MODE_4:
    case ALIGN_SUBMODE::MODE_3:
    case ALIGN_SUBMODE::MODE_2:
    case ALIGN_SUBMODE::MODE_1: {
      if (operatingTime >= MODE_5_TO_0) {
        setAlignSubmode((ALIGN_SUBMODE)((int)submode - 1));
        operatingTime = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_0: {
      break;
    }
  }

  setIndicators(indicators);

  return operatingTime;
}