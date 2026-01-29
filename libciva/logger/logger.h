#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>

class Logger {
    std::ofstream file;

protected:
  Logger();

  static Logger *singleton_;

public:
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