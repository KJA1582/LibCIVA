
#ifndef INS_CONTAINER_H
#define INS_CONTAINER_H

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

#include <fstream>
#include <functional>
#include <memory>

#include "ins/ins.h"

namespace libciva {

class INSContainer {
  std::shared_ptr<INS> unit1;
  std::shared_ptr<INS> unit2;
  std::shared_ptr<INS> unit3;

#ifndef NDEBUG
  mutable std::ofstream posLog;
  mutable double posLogTime = 0;
#endif

public:
  INSContainer(VarManager &varManager, UNIT_COUNT count, UNIT_HAS_DME dme, const std::string &configBaseID, const bool hasADEU,
               const bool hasExtendedBattery) noexcept;

#ifndef NDEBUG
  inline ~INSContainer() noexcept { posLog.close(); }
#endif

  void update(const double dTime) const noexcept;

  inline void handleEvent(std::function<void(std::shared_ptr<INS>, std::shared_ptr<INS>, std::shared_ptr<INS>)> callback) {
    callback(unit1, unit2, unit3);
  }
};

} // namespace libciva

#endif