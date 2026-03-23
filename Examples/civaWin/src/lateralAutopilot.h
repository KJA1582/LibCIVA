#ifndef LATERAL_AUTOPILOT_H
#define LATERAL_AUTOPILOT_H

#include <cmath>
#include <cstdint>

#include <libciva.h>

namespace libciva {

class LateralAutopilot {
public:
  static constexpr double MAX_BANK = 30.0;
  static constexpr double MAX_BANK_RATE = 10.0;
  static constexpr double INTERCEPT_ANGLE = 45.0;
  static constexpr double XTE_FACTOR = 30.0;
  static constexpr double REVERSE_COURSE_THRESHOLD = 90.0;

  static constexpr double INNER_KP = 1000.0;
  static constexpr double INNER_KI = 5.0;
  static constexpr double INNER_KD = 100.0;

  LateralAutopilot() noexcept : enabled(false), desiredBank(0), prevDesiredBank(0), prevError(0), integral(0) {}

  void update(const double dTime, const double bankAngle, const double rollRate, const double track, const double xte,
              const double trackAngleError, const double desiredTrack) noexcept {
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
      double trackError = deltaAngle(interceptHeading, track);
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
      // STEP 5: CHOOSE LARGER MAGNITUDE (Option B)
      //   Use the bank command with larger absolute value
      //   Sign is preserved from whichever is larger
      // =========================================================================
      if (std::fabs(interceptBank) >= std::fabs(xteBank)) {
        desiredBank = interceptBank;
      } else {
        desiredBank = xteBank;
      }
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
    integral = std::fmax(-5000, std::fmin(5000, integral));

    const double derivative = (error - prevError) / dTime;
    prevError = error;

    double rawOutput = INNER_KP * error + INNER_KI * integral + INNER_KD * derivative;
    rawOutput = std::fmax(-16383.0, std::fmin(16384.0, rawOutput));

    output = static_cast<int16_t>(std::round(rawOutput));
  }

  void enable() noexcept { enabled = true; }
  void disable() noexcept { enabled = false; }
  bool isEnabled() const noexcept { return enabled; }
  double getDesiredBank() const noexcept { return desiredBank; }
  int16_t getOutput() const noexcept { return output; }

private:
  bool enabled;
  double desiredBank;
  double prevDesiredBank;
  double prevError;
  double integral;
  int16_t output;
};

} // namespace libciva

#endif