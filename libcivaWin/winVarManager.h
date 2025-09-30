#ifndef WIN_VAR_MANAGER_H
#define WIN_VAR_MANAGER_H

#include <bitset>
#include <iomanip>
#include <iostream>

#include <libciva.h>

class WinVarManager: public VarManager {
public:
  WinVarManager() noexcept : VarManager("") { }

  void dump() const noexcept;
};

#endif