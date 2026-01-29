#include "logger/logger.h"

Logger *Logger::singleton_ = nullptr;;

Logger::Logger() {
  file = std::ofstream("log.txt", std::ios::boolalpha);
}

Logger &Logger::GetInstance() noexcept {
  if (singleton_ == nullptr) {
    singleton_ = new Logger();
  }

  return *singleton_;
}
