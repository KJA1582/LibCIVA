#ifndef WIN_VAR_MANAGER_H
#define WIN_VAR_MANAGER_H

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

#include <bitset>
#include <iomanip>
#include <iostream>

#include <libciva.h>

class WinVarManager : public libciva::VarManager {
public:
  WinVarManager() noexcept;

  void dump() const noexcept;
};

#endif