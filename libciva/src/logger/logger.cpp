#include "logger/logger.h"

Logger *Logger::singleton_ = nullptr;

Logger::Logger() noexcept { file = std::ofstream(WORK_DIR "/log.txt", std::ios::trunc); }

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

#ifndef NDEBUG

DataLogger *DataLogger::singleton_ = nullptr;

DataLogger::DataLogger() noexcept {
  file = std::ofstream(WORK_DIR "/data.csv", std::ios::trunc);
  file << std::setprecision(10);
  file << "Unit;Corrected;Initial;Mixed;Actual\n";
}

DataLogger::~DataLogger() noexcept {
  file.close();
  singleton_ = nullptr;
}

DataLogger &DataLogger::GetInstance() noexcept {
  if (singleton_ == nullptr) {
    singleton_ = new DataLogger();
  }

  return *singleton_;
}

#endif