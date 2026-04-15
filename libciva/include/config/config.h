#ifndef CONFIG_H
#define CONFIG_H

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

#include <cstring>
#include <fstream>
#include <random>
#include <string>

#include "../types/types.h"

namespace libciva {

class Config {
  const std::string basePath;
  const std::string id;

  POSITION lastINSPosition = {999, 999};
  DME lastDMEs[9] = {{{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0},
                     {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}, {{0, 0}, 0, 0}};

public:
  Config(const std::string &workDir, const std::string &id) noexcept;
  inline ~Config() noexcept { save(); }

  void save() const noexcept;

  inline POSITION getLastINSPosition() const noexcept { return lastINSPosition; }
  inline void getLastDMEs(DME (&DMEs)[9]) const noexcept { std::memcpy(DMEs, lastDMEs, sizeof(lastDMEs)); }

  inline void setLastINSPosition(POSITION pos) noexcept { lastINSPosition = pos; }
  inline void setLastDMEs(const DME (&DMEs)[9]) noexcept { std::memcpy(lastDMEs, DMEs, sizeof(DMEs)); }
};

} // namespace libciva

#endif
