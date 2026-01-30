#ifndef LIBCIVA_H
#define LIBCIVA_H

#ifndef __INTELLISENSE__
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif

#include "config/config.h"
#include "ins/ins.h"
#include "logger/logger.h"
#include "types/types.h"
#include "varManager/varManager.h"

#endif