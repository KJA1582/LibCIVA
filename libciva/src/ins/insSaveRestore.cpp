#include "ins/ins.h"

namespace libciva {

Snapshot::UnitData INS::save() const noexcept {
  Snapshot::UnitData data = {};

  data.state = static_cast<uint8_t>(state);
  data.alignSubmode = static_cast<uint8_t>(alignSubmode);
  data.batteryTest = static_cast<uint8_t>(batteryTest);
  data.dataSelector = static_cast<uint8_t>(dataSelector);
  data.modeSelector = static_cast<uint8_t>(modeSelector);
  data.insertMode = static_cast<int8_t>(insertMode);
  data.activePerformanceIndex = static_cast<uint8_t>(activePerformanceIndex);
  data.accuracyIndex = accuracyIndex;
  data.charactersRead = charactersRead;
  data.waypointSelector = waypointSelector;
  data.displayActionMalfunctionCodeIndex = displayActionMalfunctionCodeIndex;
  data.currentLegStart = currentLegStart;
  data.currentLegEnd = currentLegEnd;
  data.activeDME = activeDME;
  data.dmeMode = static_cast<int8_t>(dmeMode);
  data.valid = static_cast<uint8_t>(valid);
  data.actionMalfunctionCodes = static_cast<uint8_t>(actionMalfunctionCodes.value);
  data.indicators = indicators.value;
  data.previousIndicators = previousIndicators.value;

  data.displayPosition = displayPosition;
  data.initialINSPosition = initialINSPosition;
  data.currentINSPosition = currentINSPosition;
  data.currentTripleMixPosition = currentTripleMixPosition;
  data.holdINSPosition = holdINSPosition;
  data.holdPosition = holdPosition;
  data.simPosDelta = simPosDelta;
  for (uint8_t i = 0; i < 10; ++i) {
    data.waypoints[i] = waypoints[i];
  }
  for (uint8_t i = 0; i < 9; ++i) {
    data.dmes[i] = DMEs[i];
  }

  data.display = display.value;

  uint64_t flags = 0;
  flags |= (malfunctionCodeDisplayed ? 1 : 0) << Snapshot::FLAG_MALFUNCTION_CODE_DISPLAYED;
  flags |= (inTestMode ? 1 : 0) << Snapshot::FLAG_IN_TEST_MODE;
  flags |= (inHoldMode ? 1 : 0) << Snapshot::FLAG_IN_HOLD_MODE;
  flags |= (holdRequiresForce ? 1 : 0) << Snapshot::FLAG_HOLD_REQUIRES_FORCE;
  flags |= (autoMode ? 1 : 0) << Snapshot::FLAG_AUTO_MODE;
  flags |= (autoModePassed ? 1 : 0) << Snapshot::FLAG_AUTO_MODE_PASSED;
  flags |= (hasFacedToWaypoint ? 1 : 0) << Snapshot::FLAG_HAS_FACED_TO_WAYPOINT;
  flags |= (hasADEU ? 1 : 0) << Snapshot::FLAG_HAS_ADEU;
  flags |= (hasDME ? 1 : 0) << Snapshot::FLAG_HAS_DME;
  flags |= (dmeArmed ? 1 : 0) << Snapshot::FLAG_DME_ARMED;
  flags |= (dmeUpdating ? 1 : 0) << Snapshot::FLAG_DME_UPDATING;
  flags |= (remoteActive ? 1 : 0) << Snapshot::FLAG_REMOTE_ACTIVE;
  flags |= (hasExpandedBattery ? 1 : 0) << Snapshot::FLAG_HAS_EXPANDED_BATTERY;
  flags |= (externalPower ? 1 : 0) << Snapshot::FLAG_EXTERNAL_POWER;
  data.flags = flags;

  data.batteryRuntime = batteryRuntime;
  data.ovenTemperature = ovenTemperature;
  data.timeInMode = timeInMode;
  data.radialScalarAlignTime = radialScalarAlignTime;
  data.timeInLeg = timeInLeg;
  data.timeInDME = timeInDME;
  data.groundSpeed = groundSpeed;
  data.track = track;
  data.crossTrackError = crossTrackError;
  data.desiredTrack = desiredTrack;
  data.trackAngleError = trackAngleError;
  data.remainingDistance = remainingDistance;
  data.initialDistanceError = initialDistanceError;
  data.currentDistanceError = currentDistanceError;
  data.initialRadialError = initialRadialError;
  data.currentRadialError = currentRadialError;
  data.radialDriftPerSecond = radialDriftPerSecond;
  data.baseRadialDriftPerSecond = baseRadialDriftPerSecond;
  data.distanceDriftPerSecond = distanceDriftPerSecond;
  data.speedDriftPerSecond = speedDriftPerSecond;
  data.mixEaseTimer = mixEaseTimer;
  data.flashTimer = flashTimer;
  data.displayTimer = displayTimer;

  return data;
}

void INS::restore(const Snapshot::UnitData &data) noexcept {
  state = static_cast<INS_STATE>(data.state);
  alignSubmode = static_cast<ALIGN_SUBMODE>(data.alignSubmode);
  batteryTest = static_cast<BATTERY_TEST>(data.batteryTest);
  dataSelector = static_cast<DATA_SELECTOR>(data.dataSelector);
  modeSelector = static_cast<MODE_SELECTOR>(data.modeSelector);
  insertMode = static_cast<INSERT_MODE>(data.insertMode);
  activePerformanceIndex = static_cast<PERFORMANCE_INDEX>(data.activePerformanceIndex);
  accuracyIndex = data.accuracyIndex;
  charactersRead = data.charactersRead;
  waypointSelector = data.waypointSelector;
  displayActionMalfunctionCodeIndex = data.displayActionMalfunctionCodeIndex;
  currentLegStart = data.currentLegStart;
  currentLegEnd = data.currentLegEnd;
  activeDME = data.activeDME;
  dmeMode = static_cast<DME_MODE>(data.dmeMode);
  valid = static_cast<SIGNAL_VALIDITY>(data.valid);
  actionMalfunctionCodes.value = data.actionMalfunctionCodes;
  indicators.value = data.indicators;
  previousIndicators.value = data.previousIndicators;

  displayPosition = data.displayPosition;
  initialINSPosition = data.initialINSPosition;
  currentINSPosition = data.currentINSPosition;
  currentTripleMixPosition = data.currentTripleMixPosition;
  holdINSPosition = data.holdINSPosition;
  holdPosition = data.holdPosition;
  simPosDelta = data.simPosDelta;
  for (uint8_t i = 0; i < 10; ++i) {
    waypoints[i] = data.waypoints[i];
  }
  for (uint8_t i = 0; i < 9; ++i) {
    DMEs[i] = data.dmes[i];
  }

  display.value = data.display;

  uint64_t flags = data.flags;
  malfunctionCodeDisplayed = (flags >> Snapshot::FLAG_MALFUNCTION_CODE_DISPLAYED) & 1;
  inTestMode = (flags >> Snapshot::FLAG_IN_TEST_MODE) & 1;
  inHoldMode = (flags >> Snapshot::FLAG_IN_HOLD_MODE) & 1;
  holdRequiresForce = (flags >> Snapshot::FLAG_HOLD_REQUIRES_FORCE) & 1;
  autoMode = (flags >> Snapshot::FLAG_AUTO_MODE) & 1;
  autoModePassed = (flags >> Snapshot::FLAG_AUTO_MODE_PASSED) & 1;
  hasFacedToWaypoint = (flags >> Snapshot::FLAG_HAS_FACED_TO_WAYPOINT) & 1;
  hasADEU = (flags >> Snapshot::FLAG_HAS_ADEU) & 1;
  hasDME = (flags >> Snapshot::FLAG_HAS_DME) & 1;
  dmeArmed = (flags >> Snapshot::FLAG_DME_ARMED) & 1;
  dmeUpdating = (flags >> Snapshot::FLAG_DME_UPDATING) & 1;
  remoteActive = (flags >> Snapshot::FLAG_REMOTE_ACTIVE) & 1;
  hasExpandedBattery = (flags >> Snapshot::FLAG_HAS_EXPANDED_BATTERY) & 1;
  externalPower = (flags >> Snapshot::FLAG_EXTERNAL_POWER) & 1;

  batteryRuntime = data.batteryRuntime;
  ovenTemperature = data.ovenTemperature;
  timeInMode = data.timeInMode;
  radialScalarAlignTime = data.radialScalarAlignTime;
  timeInLeg = data.timeInLeg;
  timeInDME = data.timeInDME;
  groundSpeed = data.groundSpeed;
  track = data.track;
  crossTrackError = data.crossTrackError;
  desiredTrack = data.desiredTrack;
  trackAngleError = data.trackAngleError;
  remainingDistance = data.remainingDistance;
  initialDistanceError = data.initialDistanceError;
  currentDistanceError = data.currentDistanceError;
  initialRadialError = data.initialRadialError;
  currentRadialError = data.currentRadialError;
  radialDriftPerSecond = data.radialDriftPerSecond;
  baseRadialDriftPerSecond = data.baseRadialDriftPerSecond;
  distanceDriftPerSecond = data.distanceDriftPerSecond;
  speedDriftPerSecond = data.speedDriftPerSecond;
  mixEaseTimer = data.mixEaseTimer;
  flashTimer = data.flashTimer;
  displayTimer = data.displayTimer;

  config->setLastINSPosition(currentINSPosition);
  config->setLastDMEs(DMEs);
  config->save();
}

} // namespace libciva