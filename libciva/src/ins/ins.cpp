#include "ins/ins.h"

namespace libciva {

#pragma region Lifecycle

INS::INS(VarManager &varManager, const uint8_t id, const std::string &configID, const std::string &workDir, const bool hasADEU,
         const bool hasDME, const bool hasExpandedBattery) noexcept
    : varManager(varManager), unitIndex(id), actionMalfunctionCodes(), hasADEU(hasADEU), hasDME(hasDME),
      hasExpandedBattery(hasExpandedBattery) {
  clearDisplay();

  ovenTemperature = varManager.sim.ambientTemperature;

  config = std::make_unique<Config>(workDir, configID);
  randomGen = std::make_unique<std::mt19937>((std::random_device())());
  distributionRadial = std::make_unique<std::normal_distribution<>>(0, 0.01 / 3);
  distributionDistance = std::make_unique<std::normal_distribution<>>(0);

  if (hasExpandedBattery) batteryRuntime = EXPANDED_BATTERY_DURATION;

  // Init exports
  exportVars();
}

INS::~INS() noexcept {
  // If we have a valid position, save
  if (currentINSPosition.isValid()) {
    config->setLastINSPosition(currentINSPosition);
  }
  config->setLastDMEs(DMEs);
}

#pragma endregion

void INS::temperatureBatterySim(const double dTime) noexcept {
  // Battery
  if (externalPower) {
    batteryRuntime = std::min((double)(hasExpandedBattery ? EXPANDED_BATTERY_DURATION : BATTERY_DURATION),
                              batteryRuntime + CHARGE_RATE * dTime);
    if (batteryTest != BATTERY_TEST::RUNNING) indicators.indicator.CDU_BAT = false;
  } else if (state > INS_STATE::OFF && !externalPower) {
    batteryRuntime = std::max(0.0, batteryRuntime - dTime);
    if (batteryTest != BATTERY_TEST::RUNNING) indicators.indicator.CDU_BAT = true;
  }

  // Oven
  bool shouldHeat = state > INS_STATE::OFF;
  double ambient = varManager.sim.ambientTemperature;
  // Exit if at operating tem or ambient in case of heating or no heating
  if (shouldHeat && ovenTemperature >= OPERATING_TEMP) return;

  double cooling = shouldHeat ? 0.0 : 0.02;
  double loss = cooling * (ovenTemperature - ambient) * (dTime / UNIT_MASS);

  // Ambient following
  if (!shouldHeat) {
    ovenTemperature -= loss;
    return;
  }

  double energy = HEATER_WATTAGE * HEATER_EFFICIENCY * dTime;
  double dTemp = energy / (UNIT_MASS * UNIT_SPECIFIC_HEAT);

  ovenTemperature = ovenTemperature + dTemp - loss;
}

void INS::reset(const bool full) noexcept {
  if (full) {
    clearDisplay();
    indicators = {0};
    displayPosition = currentINSPosition = initialINSPosition = {999, 999};
    track = 0;
    valid = SIGNAL_VALIDITY::INV;

    for (uint8_t i = 0; i < 10; i++) {
      waypoints[i] = {0, 0};
    }

    insertMode = INSERT_MODE::INV;
    malfunctionCodeDisplayed = inTestMode = remoteActive = false;
    dmeMode = DME_MODE::INV;

    actionMalfunctionCodes.value = 0;
    displayActionMalfunctionCodeIndex = 0;
  }

  batteryTest = BATTERY_TEST::IDLE;
  alignSubmode = ALIGN_SUBMODE::MODE_9;
  accuracyIndex = 9;
  timeInMode = 0;
  radialScalarAlignTime = MAX_RADIAL_ERROR_SCALAR_ALIGN_TIME;
  initialDistanceError = currentDistanceError = 0;
  indicators.indicator.READY_NAV = false;
  currentTripleMixPosition = {999, 999};
  dmeArmed = dmeUpdating = false;
  activeDME = 0;
  activePerformanceIndex = 5;
}

void INS::handleOutOfBounds() noexcept {
  double gs = varManager.sim.groundVelocity;
  double trueHeading = varManager.sim.planeHeadingDegreesTrue;

  if (absDeltaAngle(trueHeading, track) > MAX_DRIFT_ANGLE) {
    actionMalfunctionCodes.codes.A02_42 = true;
    advanceActionMalfunctionIndex();
    indicators.indicator.WARN = true;
  }
  if (gs > MAX_GS) {
    actionMalfunctionCodes.codes.A02_31 = true;
    advanceActionMalfunctionIndex();
    indicators.indicator.WARN = true;
  }

  if (initialINSPosition.isValid() && state == INS_STATE::ALIGN) {
    // Inter system compare trigger 04-43
    if (unit2 && unit2->initialINSPosition.isValid() && unit2->initialINSPosition.distanceTo(displayPosition) > 0.0001) {
      actionMalfunctionCodes.codes.A04_43 = true;
      advanceActionMalfunctionIndex();
      indicators.indicator.WARN = true;
      alignSubmode = ALIGN_SUBMODE::MODE_6;
    } else if (unit3 && unit3->initialINSPosition.isValid() && unit3->initialINSPosition.distanceTo(displayPosition) > 0.0001) {
      actionMalfunctionCodes.codes.A04_43 = true;
      advanceActionMalfunctionIndex();
      indicators.indicator.WARN = true;
      alignSubmode = ALIGN_SUBMODE::MODE_6;
    }
    // Taxi
    double simLat = varManager.sim.planeLatitude;
    double simLon = varManager.sim.planeLongitude;
    POSITION simPos = {simLat, simLon};

    if (simPos.isValid() && (simPos + simPosDelta).distanceTo(initialINSPosition) > 0.0001) {
      actionMalfunctionCodes.codes.A04_57 = true;
      indicators.indicator.WARN = true;
      advanceActionMalfunctionIndex();
      alignSubmode = ALIGN_SUBMODE::MODE_6;
    }
  }
}

void INS::dmeUpdateChecks(const double dTime) noexcept {
  // Unit 3 has no DME connection so is updating whenever 1 or 2 are updating
  if (unitIndex == 2) {
    dmeUpdating = ((unit2 && unit2->dmeUpdating) || (unit3 && unit3->dmeUpdating));
    return;
  }

  // Not armed, skip
  if (!dmeArmed) return;

  // Count time
  if (dmeArmed || dmeUpdating) {
    timeInDME += dTime;
  }

  // Not min time, skip
  if (timeInDME < MIN_DME_TIME) return;

  // Get DME value
  double dmeDist = unitIndex == 0 ? varManager.sim.navDme1 : varManager.sim.navDme2;
  bool valid = dmeDist >= 0;

  // Drop out of DME if not valid
  if (!valid || dmeDist < 0) {
    dmeArmed = dmeUpdating = false;
    activeDME = 0;
    if (unitIndex == 0) indicators.indicator.DME1 = false;
    if (unitIndex == 1) indicators.indicator.DME2 = false;
  }

  // Check distance, drop out if not reasonable
  DME dme = DMEs[activeDME - 1];

  double alt = varManager.sim.planeAltitude;

  double gcDist = dme.position.distanceTo((currentINSPosition));
  double slantCorrectedDMEDist = std::sqrt(std::pow(dmeDist, 2) - std::pow((alt - dme.altitude) * 0.000164579, 2));
  if (std::abs(gcDist - slantCorrectedDMEDist) > 0.5) {
    dmeArmed = dmeUpdating = false;
    activeDME = 0;
    if (unitIndex == 0) indicators.indicator.DME1 = false;
    if (unitIndex == 1) indicators.indicator.DME2 = false;
    return;
  }

  if (!dmeUpdating) {
    dmeUpdating = true;
    if (unitIndex == 0) indicators.indicator.DME1 = true;
    if (unitIndex == 1) indicators.indicator.DME2 = true;
    timeInMode = 0;
  }
}

} // namespace libciva