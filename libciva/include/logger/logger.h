#ifndef LOGGER_H
#define LOGGER_H

#ifndef __INTELLISENSE__
#	define MODULE_EXPORT __attribute__( ( visibility( "default" ) ) )
#	define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#else
#	define MODULE_EXPORT
#	define MODULE_WASM_MODNAME(mod)
#	define __attribute__(x)
#	define __restrict__
#endif

#include <fstream>

class Logger {
    std::ofstream file;

protected:
  Logger() noexcept;

  static Logger *singleton_;

public:
  ~Logger() noexcept;

  Logger(Logger &other) = delete;
  void operator=(const Logger &) = delete;

  template<typename T>
  Logger &operator<<(const T &val) {
    this->file << val;
    file.flush();

    return *this;
  }

  static Logger &GetInstance() noexcept;
};

#endif