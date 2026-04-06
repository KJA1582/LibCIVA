#include "verticalAutopilot.h"

VerticalAutopilot::VerticalAutopilot() noexcept
    : enabled(false), desiredPitch(0), prevDesiredPitch(0), prevError(0), integral(0), timestamp(0), targetAltitude(0)
#ifndef NDEBUG
      ,
      logFile(nullptr)
#endif
{
#ifndef NDEBUG
  logFile = std::fopen("vertical_autopilot_log.csv", "w");
  if (logFile) {
    std::fprintf(logFile, "timestamp,targetAltitude,altitude,altitudeError,desiredPitch,pitchAngle,pitchRate,elevatorOutput\n");
  }
#endif
}

VerticalAutopilot::~VerticalAutopilot() noexcept {
#ifndef NDEBUG
  if (logFile) {
    std::fflush(logFile);
    std::fclose(logFile);
    logFile = nullptr;
  }
#endif
}

void VerticalAutopilot::update(const double dTime, const double altitude, const double pitchAngle,
                               const double pitchRate) noexcept {
  timestamp += dTime;

  // =========================================================================
  // SIGN CONVENTIONS:
  //   PLANE_PITCH_DEGREES (sim input):  negative = nose UP,  positive = nose DOWN
  //   ROTATION VELOCITY BODY X (sim):    negative = pitch rate UP, positive = pitch rate DOWN
  //   ELEVATOR_SET (output to sim):      negative = nose DOWN, positive = nose UP
  //
  //   RELATIONSHIP: Our internal convention uses positive = pitch UP
  //                 negative pitchAngle in sim -> positive (our convention)
  //                 negative elevator in sim -> nose DOWN
  // =========================================================================

  if (!enabled) {
    output = 0;
    return;
  }

  // =========================================================================
  // STEP 1: CONVERT SIGN CONVENTIONS
  //   Negate pitchAngle: sim negative (nose UP) -> our positive (pitch UP)
  //   Negate pitchRate:  sim negative (rate UP) -> our positive (rate UP)
  // =========================================================================
  const double negPitchAngle = -pitchAngle;
  const double negPitchRate = -pitchRate;

#ifndef NDEBUG
  if (logFile) {
    double altitudeError = targetAltitude - altitude;
    std::fprintf(logFile, "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d\n", timestamp, targetAltitude, altitude, altitudeError,
                 desiredPitch, negPitchAngle, negPitchRate, static_cast<int>(output));
  }
#endif

  // =========================================================================
  // STEP 2: OUTER LOOP - ALTITUDE ERROR TO DESIRED PITCH
  //   Calculate altitude error: targetAltitude - currentAltitude
  //   positive error = below target = want to pitch UP
  //   Convert error to desired pitch angle using ALT_HOLD_KP
  //   Clamp to MAX_PITCH (10 degrees max)
  // =========================================================================
  double altitudeError = targetAltitude - altitude;
  desiredPitch = altitudeError * ALT_HOLD_KP;
  desiredPitch = std::fmax(-MAX_PITCH, std::fmin(MAX_PITCH, desiredPitch));

  // =========================================================================
  // STEP 3: RATE LIMITING
  //   Limit rate of change to MAX_PITCH_RATE degrees per second (5 deg/s max)
  // =========================================================================
  double maxChange = MAX_PITCH_RATE * dTime;
  if (desiredPitch > prevDesiredPitch + maxChange) {
    desiredPitch = prevDesiredPitch + maxChange;
  } else if (desiredPitch < prevDesiredPitch - maxChange) {
    desiredPitch = prevDesiredPitch - maxChange;
  }
  prevDesiredPitch = desiredPitch;

  // =========================================================================
  // STEP 4: INNER PID LOOP - Drive actual pitch to desired pitch
  //   Error: difference between desired and actual pitch
  //   Output will be scaled to elevator range (-16383 to 16384)
  //   Note: negative elevator = nose DOWN, so we negate the output
  // =========================================================================
  const double error = desiredPitch - negPitchAngle;

  integral += error * dTime;
  integral = std::fmax(-5000.0, std::fmin(5000.0, integral));

  const double derivative = (error - prevError) / dTime;
  prevError = error;

  double rawOutput = INNER_KP * error + INNER_KI * integral + INNER_KD * derivative;
  rawOutput = std::fmax(-16383.0, std::fmin(16384.0, rawOutput));

  output = static_cast<int16_t>(std::round(-rawOutput));
}

void VerticalAutopilot::enable(const double currentAltitude) noexcept {
  enabled = true;
  targetAltitude = currentAltitude;
  integral = 0;
  prevError = 0;
  desiredPitch = 0;
  prevDesiredPitch = 0;
}

void VerticalAutopilot::disable() noexcept { enabled = false; }