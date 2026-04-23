
#ifndef INS_CONTAINER_H
#define INS_CONTAINER_H

// For MSFS so IntelliSense doesn't crap out
#ifndef __INTELLISENSE__
#ifndef MODULE_EXPORT
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#endif
#define CIVA_EXPORT __attribute__((visibility("default")))
#else
#if defined(WIN32) && defined(SHARED)
#define CIVA_EXPORT __declspec(dllexport)
#elif defined(SHARED)
#define CIVA_EXPORT __attribute__((visibility("default")))
#else
#define CIVA_EXPORT
#endif
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include <fstream>
#include <functional>
#include <memory>

#include "ins/ins.h"
#include "state/snapshot.h"

namespace libciva {

class CIVA_EXPORT INSContainer {
  std::shared_ptr<INS> unit1;
  std::shared_ptr<INS> unit2;
  std::shared_ptr<INS> unit3;

#ifndef NDEBUG
  mutable std::ofstream posLog;
  mutable double posLogTime = 0;
#endif

public:
  inline INSContainer(VarManager &varManager, UNIT_COUNT count, UNIT_HAS_DME dme, const std::string &configBaseID,
                      const bool hasADEU, const bool hasExtendedBattery) noexcept {
    unit1 = std::make_shared<INS>(varManager, UNIT_INDEX::UNIT_1, configBaseID + "_1", WORK_DIR, hasADEU,
                                  dme == UNIT_HAS_DME::ONE || dme == UNIT_HAS_DME::BOTH, hasExtendedBattery);

    if (count > UNIT_COUNT::ONE)
      unit2 = std::make_shared<INS>(varManager, UNIT_INDEX::UNIT_2, configBaseID + "_2", WORK_DIR, hasADEU,
                                    dme == UNIT_HAS_DME::TWO || dme == UNIT_HAS_DME::BOTH, hasExtendedBattery);
    if (count == UNIT_COUNT::THREE)
      unit3 = std::make_shared<INS>(varManager, UNIT_INDEX::UNIT_3, configBaseID + "_3", WORK_DIR, hasADEU, false,
                                    hasExtendedBattery);

    if (count > UNIT_COUNT::ONE) unit1->connectUnit2(unit2.get());
    if (count == UNIT_COUNT::THREE) unit1->connectUnit3(unit3.get());

    if (count > UNIT_COUNT::ONE) unit2->connectUnit2(unit1.get());
    if (count == UNIT_COUNT::THREE) unit2->connectUnit3(unit3.get());

    if (count == UNIT_COUNT::THREE) unit3->connectUnit2(unit1.get());
    if (count == UNIT_COUNT::THREE) unit3->connectUnit3(unit2.get());

#ifndef NDEBUG
    posLog = std::ofstream(WORK_DIR "/pos_log.csv", std::ios::trunc);
#endif
  }

#ifndef NDEBUG
  inline ~INSContainer() noexcept { posLog.close(); }
#endif

  inline void update(const double dTime) const noexcept {
    unit1->updatePreMix(dTime);
    if (unit2) unit2->updatePreMix(dTime);
    if (unit3) unit3->updatePreMix(dTime);

    unit1->updateMix();
    if (unit2) unit2->updateMix();
    if (unit3) unit3->updateMix();

    unit1->updatePostMix(dTime);
    if (unit2) unit2->updatePostMix(dTime);
    if (unit3) unit3->updatePostMix(dTime);

#ifndef NDEBUG
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
             << unit1->varManager.sim.planeLongitude << ";";
      if (unit1->dmeUpdating || unit2->dmeUpdating)
        posLog << "1"
               << "\n";
      else
        posLog << "0"
               << "\n";

      posLog.flush();
    }
#endif
  }

  inline void
  handleEvent(std::function<void(std::shared_ptr<INS>, std::shared_ptr<INS>, std::shared_ptr<INS>)> callback) const noexcept {
    callback(unit1, unit2, unit3);
  }

  inline void remoteInsert(const POSITION (&wpts)[9], const DME (&dmes)[9]) const noexcept {
    if (unit1) {
      unit1->remoteInsertWPT(wpts);
      unit1->remoteInsertDME(dmes);
    }
    if (unit2) {
      unit2->remoteInsertWPT(wpts);
      unit2->remoteInsertDME(dmes);
    }
    if (unit3) {
      unit3->remoteInsertWPT(wpts);
      unit3->remoteInsertDME(dmes);
    }
  }

  inline void saveState(Snapshot &snapshot) const noexcept {
    snapshot.version = Snapshot::LIBCIVA_SNAPSHOT_VERSION;
    snapshot.units.clear();
    snapshot.units.push_back(unit1->save());
    if (unit2) snapshot.units.push_back(unit2->save());
    if (unit3) snapshot.units.push_back(unit3->save());
    snapshot.unitCount = static_cast<uint8_t>(snapshot.units.size());
  }

  inline bool restoreState(const Snapshot &snapshot) noexcept {
    if (snapshot.version != Snapshot::LIBCIVA_SNAPSHOT_VERSION) return false;
    if (snapshot.units.empty()) return false;

    unit1->restore(snapshot.units[0]);
    if (unit2 && snapshot.units.size() > 1) unit2->restore(snapshot.units[1]);
    if (unit3 && snapshot.units.size() > 2) unit3->restore(snapshot.units[2]);

    return true;
  }
};

} // namespace libciva

#endif