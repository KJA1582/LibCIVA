#ifndef LOGGER_H
#define LOGGER_H

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

#include <ctime>
#include <fstream>
#include <iomanip>

namespace libciva {

class Logger {
protected:
  std::ofstream file;

  Logger() noexcept;

public:
  ~Logger() noexcept;

  Logger(Logger &other) = delete;
  void operator=(const Logger &) = delete;

  template <typename T> inline Logger &operator<<(const T &val) noexcept {
    this->file << val;
    file.flush();

    return *this;
  }
  inline Logger &operator<<(Logger &) noexcept { return *this; }

  inline Logger &time() noexcept {
    std::time_t t = std::time(nullptr);
#pragma warning(push)
#pragma warning(disable : 4996)
    std::tm tm = *std::localtime(&t);
#pragma warning(pop)

    this->file << std::put_time(&tm, "%FT%TZ: ");
    file.flush();

    return *this;
  }

  static Logger &GetInstance() noexcept;
};

} // namespace libciva

#endif