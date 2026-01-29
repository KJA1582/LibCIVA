#ifndef MSFS_VAR_MANAGER_H
#define MSFS_VAR_MANAGER_H

#ifndef __INTELLISENSE__
#	define MODULE_EXPORT __attribute__( ( visibility( "default" ) ) )
#	define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#	define MODULE_EXPORT
#	define MODULE_WASM_MODNAME(mod)
#	define __attribute__(x)
#	define __restrict__
#endif

#include <libciva.h>

class MSFSVarManager: public VarManager {
public:
  MSFSVarManager() noexcept;
};

#endif