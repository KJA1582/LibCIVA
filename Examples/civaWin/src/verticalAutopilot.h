#ifndef VERTICAL_AUTOPILOT_H
#define VERTICAL_AUTOPILOT_H

#include <cmath>
#include <cstdint>
#include <cstdio>

#include <libciva.h>

namespace libciva {

class VerticalAutopilot {
public:
  static constexpr double MAX_PITCH = 10.0;
  static constexpr double MAX_PITCH_RATE = 5.0;
  static constexpr double ALT_HOLD_KP = 0.5;

  static constexpr double INNER_KP = 400.0;
  static constexpr double INNER_KI = 5.0;
  static constexpr double INNER_KD = 300.0;

  static constexpr const char *LOG_FILENAME = "vertical_autopilot_log.csv";

  VerticalAutopilot() noexcept
      : enabled(false), desiredPitch(0), prevDesiredPitch(0), prevError(0), integral(0), timestamp(0), targetAltitude(0),
        logFile(nullptr) {
    logFile = std::fopen(LOG_FILENAME, "w");
    if (logFile) {
      std::fprintf(logFile, "timestamp,targetAltitude,altitude,altitudeError,desiredPitch,pitchAngle,pitchRate,elevatorOutput\n");
    }
  }

  ~VerticalAutopilot() noexcept {
    if (logFile) {
      std::fflush(logFile);
      std::fclose(logFile);
      logFile = nullptr;
    }
  }

  void update(const double dTime, const double altitude, const double pitchAngle, const double pitchRate) noexcept {
    if (!enabled) {
      output = 0;
      return;
    }

    const double negPitchAngle = -pitchAngle;
    const double negPitchRate = -pitchRate;

    if (logFile) {
      double altitudeError = targetAltitude - altitude;
      std::fprintf(logFile, "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d\n", timestamp, targetAltitude, altitude, altitudeError,
                   desiredPitch, negPitchAngle, negPitchRate, static_cast<int>(output));
    }

    timestamp += dTime;

    double altitudeError = targetAltitude - altitude;
    desiredPitch = altitudeError * ALT_HOLD_KP;
    desiredPitch = std::fmax(-MAX_PITCH, std::fmin(MAX_PITCH, desiredPitch));

    double maxChange = MAX_PITCH_RATE * dTime;
    if (desiredPitch > prevDesiredPitch + maxChange) {
      desiredPitch = prevDesiredPitch + maxChange;
    } else if (desiredPitch < prevDesiredPitch - maxChange) {
      desiredPitch = prevDesiredPitch - maxChange;
    }
    prevDesiredPitch = desiredPitch;

    const double error = desiredPitch - negPitchAngle;

    integral += error * dTime;
    integral = std::fmax(-5000, std::fmin(5000, integral));

    const double derivative = (error - prevError) / dTime;
    prevError = error;

    double rawOutput = INNER_KP * error + INNER_KI * integral + INNER_KD * derivative;
    rawOutput = std::fmax(-16383.0, std::fmin(16384.0, rawOutput));

    output = static_cast<int16_t>(std::round(-rawOutput));
  }

  void enable(const double currentAltitude) noexcept {
    enabled = true;
    timestamp = 0;
    targetAltitude = currentAltitude;
    integral = 0;
    prevError = 0;
    desiredPitch = 0;
    prevDesiredPitch = 0;
  }

  void disable() noexcept {
    enabled = false;
    timestamp = 0;
  }

  bool isEnabled() const noexcept { return enabled; }
  double getTargetAltitude() const noexcept { return targetAltitude; }
  double getDesiredPitch() const noexcept { return desiredPitch; }
  int16_t getOutput() const noexcept { return output; }

private:
  bool enabled;
  double desiredPitch;
  double prevDesiredPitch;
  double prevError;
  double integral;
  int16_t output;
  double timestamp;
  double targetAltitude;
  FILE *logFile;
};

} // namespace libciva

#endif
