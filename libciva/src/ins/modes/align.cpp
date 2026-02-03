#include "ins/ins.h"

void INS::align(const double dTime) noexcept {
  if (modeSelector == MODE_SELECTOR::STBY) {
    // Downmode
    state = INS_STATE::STBY;
    reset(false);
    actionMalfunctionCodes.value = 0;

    return;
  } else if (modeSelector == MODE_SELECTOR::NAV) {
    // Upmode
    if (alignSubmode <= ALIGN_SUBMODE::MODE_5) {
      state = INS_STATE::NAV;
      indicators.indicator.READY_NAV = false;
      accuracyIndex = 0;
      radialError = 0;
      // If you enter exactly at AI5, you get double the drift (3sigma of 0.02 °/h)
      radialDriftPerSecond = baseRadialDriftPerSecond * (1 + (radialScalarAlignTime / MAX_RADIAL_ERROR_SCALAR_ALIGN_TIME));

      updateSimPosDelta();

      timeInMode = 0;
      return;
    }
  }

  switch (alignSubmode) {
    case ALIGN_SUBMODE::MODE_9: {
      if (ovenTemperature >= config.getOperatingTempInC()) {
        alignSubmode = ALIGN_SUBMODE::MODE_8;
        timeInMode = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_8: {
      if (batteryTest == BATTERY_TEST::IDLE && (uint8_t)state != (uint8_t)modeSelector) {
        batteryTest = BATTERY_TEST::INHIBITED;
      } else if (batteryTest == BATTERY_TEST::IDLE && timeInMode < MAX_BAT_TEST_TIME) {
        indicators.indicator.CDU_BAT = true;
        batteryTest = BATTERY_TEST::RUNNING;
      } else if (batteryTest == BATTERY_TEST::RUNNING && timeInMode >= MAX_BAT_TEST_TIME) {
        indicators.indicator.CDU_BAT = false;
        batteryTest = BATTERY_TEST::COMPLETED;
      }

      if (currentINSPosition.isValid()) {
        config.setLastINSPosition(currentINSPosition);
      }

      if (timeInMode >= MIN_MODE_8) {
        alignSubmode = ALIGN_SUBMODE::MODE_7;
        timeInMode = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_7: {
      if (timeInMode >= MAX_MODE_7 && currentINSPosition.isValid() && actionMalfunctionCodes.value == 0) {
        alignSubmode = ALIGN_SUBMODE::MODE_6;
        timeInMode = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_6: {
      if (timeInMode >= MAX_MODE_6 && actionMalfunctionCodes.value == 0) {
        alignSubmode = ALIGN_SUBMODE::MODE_5;

        indicators.indicator.READY_NAV = true;
        timeInMode = 0;
      }
      break;
    }
    case ALIGN_SUBMODE::MODE_5:
    case ALIGN_SUBMODE::MODE_4:
    case ALIGN_SUBMODE::MODE_3:
    case ALIGN_SUBMODE::MODE_2:
    case ALIGN_SUBMODE::MODE_1: {
      if (timeInMode >= MODE_5_TO_0) {
        alignSubmode = (ALIGN_SUBMODE)((uint8_t)alignSubmode - 1);
        timeInMode = 0;
      }
      radialScalarAlignTime =
          std::max(0.0, radialScalarAlignTime - dTime * MAX_RADIAL_ERROR_SCALAR_ALIGN_TIME / (5.0 * MODE_5_TO_0));
      break;
    }
    case ALIGN_SUBMODE::MODE_0: {
      radialScalarAlignTime = 0;
      break;
    }
  }
}