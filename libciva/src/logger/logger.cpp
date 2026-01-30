#include "logger/logger.h"

Logger *Logger::singleton_ = nullptr;

Logger::Logger() noexcept {
  file = std::ofstream(WORK_DIR "/log.txt", std::ios::trunc);
}

Logger::~Logger() noexcept {
  file.close();
  singleton_ = nullptr;
}

Logger &Logger::GetInstance() noexcept {
  if (singleton_ == nullptr) {
    singleton_ = new Logger();
  }

  return *singleton_;
}
