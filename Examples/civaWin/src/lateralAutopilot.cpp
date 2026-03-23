#include "lateralAutopilot.h"

LateralAutopilot::LateralAutopilot() noexcept
    : enabled(false), desiredBank(0), prevDesiredBank(0), prevError(0), integral(0), timestamp(0)
#ifndef NDEBUG
      ,
      logFile(nullptr)
#endif
{
#ifndef NDEBUG
  logFile = std::fopen("lateral_autopilot_log.csv", "w");
  if (logFile) {
    std::fprintf(logFile, "timestamp,desiredBank,bankAngle,xte,trackAngleError,desiredTrack,track,rollRate,aileronOutput\n");
  }
#endif
}

LateralAutopilot::~LateralAutopilot() noexcept {
#ifndef NDEBUG
  if (logFile) {
    std::fflush(logFile);
    std::fclose(logFile);
    logFile = nullptr;
  }
#endif
}

void LateralAutopilot::update(const double dTime, const double bankAngle, const double rollRate, const double track,
                              const double xte, const double trackAngleError, const double desiredTrack) noexcept {
  timestamp += dTime;

  // =========================================================================
  // SIGN CONVENTIONS:
  //   PLANE_BANK_DEGREES (sim input):  positive = left turn,  negative = right turn
  //   AILERON_SET (output to sim):     positive = right turn, negative = left turn
  //   XTE (cross track error):         positive = plane is RIGHT of track
  //   TKE (trackAngleError):           positive = TRK is LEFT of DTK
  //
  //   RELATIONSHIP: AILERON_SET sign is OPPOSITE to PLANE_BANK_DEGREES sign
  //                 positive bank angle -> negative aileron -> left turn
  // =========================================================================

  if (!enabled) {
    output = 0;
    return;
  }

  // Log data when AP is enabled
  // Note: output is AILERON_SET (positive = right turn, negative = left turn)
#ifndef NDEBUG
  if (logFile) {
    std::fprintf(logFile, "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d\n", timestamp, desiredBank, bankAngle, xte, trackAngleError,
                 desiredTrack, track, rollRate, static_cast<int>(output));
  }
#endif

  // =========================================================================
  // STEP 1: REVERSE COURSE DETECTION
  //   Use TKE to detect reverse course. If |TKE| > 90, we are on opposite course.
  //   TKE > 0: TRK is LEFT of DTK -> need right turn (negative bank, positive aileron)
  //   TKE < 0: TRK is RIGHT of DTK -> need left turn (positive bank, negative aileron)
  // =========================================================================
  if (std::fabs(trackAngleError) > REVERSE_COURSE_THRESHOLD) {
    // Reverse course - use max bank in direction indicated by TKE
    // TKE > 0: TRK left of DTK -> need right turn -> negative bank (PLANE_BANK_DEGREES)
    // TKE < 0: TRK right of DTK -> need left turn -> positive bank (PLANE_BANK_DEGREES)
    // Note: TKE positive means TRK is left of DTK, so we need right turn (negative bank)
    desiredBank = (trackAngleError >= 0) ? -MAX_BANK : MAX_BANK;
  } else {
    // =========================================================================
    // STEP 2: CALCULATE INTERCEPT HEADING
    //   Intercept angle is proportional to XTE:
    //     XTE = 0nm -> intercept angle = 0 degrees (follow DTK directly)
    //     XTE = 1nm -> intercept angle = 45 degrees
    //     XTE = 2nm -> intercept angle = 90 degrees (clamped to MAX)
    //   XTE > 0 (right of track): turn LEFT toward DTK -> DTK - interceptAngle
    //   XTE < 0 (left of track):  turn RIGHT toward DTK -> DTK + interceptAngle
    // =========================================================================
    double interceptAngle = std::fmin(INTERCEPT_ANGLE, std::fabs(xte) * INTERCEPT_ANGLE);
    double interceptHeading;
    if (xte >= 0) {
      // Right of track - turn left toward DTK
      interceptHeading = desiredTrack - interceptAngle;
    } else {
      // Left of track - turn right toward DTK
      interceptHeading = desiredTrack + interceptAngle;
    }

    // =========================================================================
    // STEP 3: CALCULATE INTERCEPT BANK
    //   trackError = angle from current TRK to interceptHeading
    //   interceptBank = -trackError
    //     - trackError > 0 (need right turn) -> negative bank (correct for right turn)
    //     - trackError < 0 (need left turn)  -> positive bank (correct for left turn)
    //   Clamp to MAX_BANK
    // =========================================================================
    double trackError = libciva::deltaAngle(interceptHeading, track);
    double interceptBank = -trackError;
    interceptBank = std::fmax(-MAX_BANK, std::fmin(MAX_BANK, interceptBank));

    // =========================================================================
    // STEP 4: CALCULATE XTE BANK
    //   xteBank = xte * XTE_FACTOR
    //     - XTE > 0 (right of track) -> positive xteBank -> positive bank -> left turn
    //     - XTE < 0 (left of track)  -> negative xteBank -> negative bank -> right turn
    //   Clamp to MAX_BANK
    // =========================================================================
    double xteBank = xte * XTE_FACTOR;
    xteBank = std::fmax(-MAX_BANK, std::fmin(MAX_BANK, xteBank));

    // =========================================================================
    // STEP 5: WEIGHTED ADDITIVE
    //   Combine intercept and XTE bank commands with weights
    //   interceptBank contributes 0.7x, xteBank contributes 1.0x
    //   Result is clamped to MAX_BANK
    // =========================================================================
    desiredBank = INTERCEPT_WEIGHT * interceptBank + xteBank;
    desiredBank = std::fmax(-MAX_BANK, std::fmin(MAX_BANK, desiredBank));
  }

  // =========================================================================
  // STEP 6: RATE LIMITING
  //   Limit rate of change to MAX_BANK_RATE degrees per second
  //   Always applied, even in reverse course
  // =========================================================================
  double maxChange = MAX_BANK_RATE * dTime;
  if (desiredBank > prevDesiredBank + maxChange) {
    desiredBank = prevDesiredBank + maxChange;
  } else if (desiredBank < prevDesiredBank - maxChange) {
    desiredBank = prevDesiredBank - maxChange;
  }
  prevDesiredBank = desiredBank;

  // =========================================================================
  // STEP 7: INNER PID LOOP - Drive actual bank to desired bank
  //   Error: difference between desired and actual bank
  //   Output will be scaled to aileron range (-16383 to 16384)
  //   Note: positive aileron = right turn, so output sign is OPPOSITE to bank sign
  // =========================================================================
  const double error = desiredBank - bankAngle;

  integral += error * dTime;
  integral = std::fmax(-5000.0, std::fmin(5000.0, integral));

  const double derivative = (error - prevError) / dTime;
  prevError = error;

  double rawOutput = INNER_KP * error + INNER_KI * integral + INNER_KD * derivative;
  rawOutput = std::fmax(-16383.0, std::fmin(16384.0, rawOutput));

  output = static_cast<int16_t>(std::round(rawOutput));
}

void LateralAutopilot::enable() noexcept {
  enabled = true;
  integral = 0;
  prevError = 0;
  desiredBank = 0;
  prevDesiredBank = 0;
}

void LateralAutopilot::disable() noexcept { enabled = false; }