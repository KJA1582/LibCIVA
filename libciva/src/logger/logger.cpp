#include "logger/logger.h"

namespace libciva {

Logger::Logger() noexcept { file = std::ofstream(WORK_DIR "/log.txt", std::ios::trunc); }

Logger::~Logger() noexcept { file.close(); }

Logger &Logger::GetInstance() noexcept {
  static Logger instance;
  return instance;
}

} // namespace libciva