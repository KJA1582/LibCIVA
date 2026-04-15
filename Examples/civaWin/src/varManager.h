#ifndef WIN_VAR_MANAGER_H
#define WIN_VAR_MANAGER_H

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

#include <bitset>
#include <iomanip>
#include <iostream>

#include <libciva.h>

class WinVarManager : public libciva::VarManager {
public:
  double simRate = 0;

  // Pure AP Demo
  double rollRate = 0;
  double bankAngle = 0;
  double pitchRate = 0;
  double pitchAngle = 0;

  WinVarManager() noexcept;

  void dump() const noexcept;
};

#endif