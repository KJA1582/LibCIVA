#ifndef LIBCIVA_H
#define LIBCIVA_H

// For MSFS so IntelliSense doesn't crap out
#ifndef __INTELLISENSE__
#ifndef MODULE_EXPORT
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#endif
#else
#ifndef MODULE_EXPORT
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif
#endif

#ifndef WORK_DIR
#define WORK_DIR "\\work"
#endif

#include "ins/ins.h"
#include "insContainer/insContainer.h"
#include "logger/logger.h"
#include "state/state.h"
#include "types/types.h"
#include "varManager/varManager.h"


#endif