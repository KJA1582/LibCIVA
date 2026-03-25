#include "insContainer/insContainer.h"

namespace libciva {

INSContainer::INSContainer(VarManager &varManager, UNIT_COUNT count, UNIT_HAS_DME dme, const std::string &configBaseID,
                           const bool hasADEU, const bool hasExtendedBattery) noexcept {
  unit1 = std::make_shared<INS>(varManager, 0, configBaseID + "_1", WORK_DIR, hasADEU,
                                dme == UNIT_HAS_DME::ONE || dme == UNIT_HAS_DME::BOTH, hasExtendedBattery);

  if (count > UNIT_COUNT::ONE)
    unit2 = std::make_shared<INS>(varManager, 1, configBaseID + "_2", WORK_DIR, hasADEU,
                                  dme == UNIT_HAS_DME::TWO || dme == UNIT_HAS_DME::BOTH, hasExtendedBattery);
  if (count == UNIT_COUNT::THREE)
    unit3 = std::make_shared<INS>(varManager, 2, configBaseID + "_3", WORK_DIR, hasADEU, false, hasExtendedBattery);

  if (count > UNIT_COUNT::ONE) unit1->connectUnit2(unit2.get());
  if (count == UNIT_COUNT::THREE) unit1->connectUnit3(unit3.get());

  if (count > UNIT_COUNT::ONE) unit2->connectUnit2(unit1.get());
  if (count == UNIT_COUNT::THREE) unit2->connectUnit3(unit3.get());

  if (count == UNIT_COUNT::THREE) unit3->connectUnit2(unit1.get());
  if (count == UNIT_COUNT::THREE) unit3->connectUnit3(unit2.get());

#ifndef NDEBUG
  posLog = std::ofstream(WORK_DIR "pos_log.csv", std::ios::trunc);
#endif
}

void INSContainer::update(const double dTime) const noexcept {
  unit1->updatePreMix(dTime);
  if (unit2) unit2->updatePreMix(dTime);
  if (unit3) unit3->updatePreMix(dTime);

  unit1->updateMix();
  if (unit2) unit2->updateMix();
  if (unit3) unit3->updateMix();

  unit1->updatePostMix(dTime);
  if (unit2) unit2->updatePostMix(dTime);
  if (unit3) unit3->updatePostMix(dTime);

  posLogTime += dTime;
  if (posLogTime >= 1.0) {
    posLogTime -= 1.0;

    posLog << std::setprecision(8) << unit1->currentINSPosition.latitude << "," << std::setprecision(9)
           << unit1->currentINSPosition.longitude << ";";
    if (unit2)
      posLog << std::setprecision(8) << unit2->currentINSPosition.latitude << "," << std::setprecision(9)
             << unit2->currentINSPosition.longitude << ";";
    else
      posLog << "999,999;";
    if (unit3)
      posLog << std::setprecision(8) << unit3->currentINSPosition.latitude << "," << std::setprecision(9)
             << unit3->currentINSPosition.longitude << ";";
    else
      posLog << "999,999;";
    posLog << std::setprecision(8) << unit1->currentTripleMixPosition.latitude << "," << std::setprecision(9)
           << unit1->currentTripleMixPosition.longitude << ";";
    posLog << std::setprecision(8) << unit1->varManager.sim.planeLatitude << "," << std::setprecision(9)
           << unit1->varManager.sim.planeLongitude << "\n";

    posLog.flush();
  }
}

} // namespace libciva