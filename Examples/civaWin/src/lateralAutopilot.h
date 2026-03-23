#ifndef LATERAL_AUTOPILOT_H
#define LATERAL_AUTOPILOT_H

// For MSFS so IntelliSense doesn't crap out
#ifndef __INTELLISENSE__
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
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

class LateralAutopilot {
private:
  bool enabled;
  double desiredBank;
  double prevDesiredBank;
  double prevError;
  double integral;
  int16_t output;
  double timestamp;
#ifndef NDEBUG
  FILE *logFile;
#endif

public:
  static constexpr double MAX_BANK = 30.0;
  static constexpr double MAX_BANK_RATE = 10.0;
  static constexpr double INTERCEPT_ANGLE = 45.0;
  static constexpr double XTE_FACTOR = 30.0;
  static constexpr double REVERSE_COURSE_THRESHOLD = 90.0;
  static constexpr double INTERCEPT_WEIGHT = 1.2;

  static constexpr double INNER_KP = 800.0;
  static constexpr double INNER_KI = 10.0;
  static constexpr double INNER_KD = 150.0;

  LateralAutopilot() noexcept;
  ~LateralAutopilot() noexcept;

  void update(const double dTime, const double bankAngle, const double rollRate, const double track, const double xte,
              const double trackAngleError, const double desiredTrack) noexcept;

  void enable() noexcept;
  void disable() noexcept;

  bool isEnabled() const noexcept { return enabled; }
  double getDesiredBank() const noexcept { return desiredBank; }
  int16_t getOutput() const noexcept { return output; }
};

#endif