#ifndef VERTICAL_AUTOPILOT_H
#define VERTICAL_AUTOPILOT_H

// For MSFS so IntelliSense doesn't crap out
#ifndef __INTELLISENSE__
#ifndef MODULE_EXPORT
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#endif
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include <cmath>
#include <cstdint>

#ifndef NDEBUG
#include <cstdio>
#endif

#include <libciva.h>

class VerticalAutopilot {
  static constexpr double MAX_PITCH = 10.0;
  static constexpr double MAX_PITCH_RATE = 5.0;
  static constexpr double ALT_HOLD_KP = 0.5;

  static constexpr double INNER_KP = 200.0;
  static constexpr double INNER_KI = 5.0;
  static constexpr double INNER_KD = 300.0;

  bool enabled;
  double desiredPitch;
  double prevDesiredPitch;
  double prevError;
  double integral;
  int16_t output;
  double timestamp;
  double targetAltitude;
#ifndef NDEBUG
  FILE *logFile;
#endif

public:
  VerticalAutopilot() noexcept;
  ~VerticalAutopilot() noexcept;

  void update(const double dTime, const double altitude, const double pitchAngle, const double pitchRate) noexcept;

  void enable(const double currentAltitude) noexcept;
  void disable() noexcept;

  bool isEnabled() const noexcept { return enabled; }
  double getTargetAltitude() const noexcept { return targetAltitude; }
  double getDesiredPitch() const noexcept { return desiredPitch; }
  int16_t getOutput() const noexcept { return output; }
};

#endif