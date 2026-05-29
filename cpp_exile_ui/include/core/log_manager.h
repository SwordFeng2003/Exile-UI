#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <memory>
#include <spdlog/spdlog.h>

namespace exile::core {

class LogManager {
public:
  static LogManager& instance();

  void initialize(const std::filesystem::path& log_path, bool enabled = true);
  void shutdown();

  template<typename... Args>
  void info(Args&&... args) {
    if (logger_) logger_->info(std::forward<Args>(args)...);
  }

  template<typename... Args>
  void warn(Args&&... args) {
    if (logger_) logger_->warn(std::forward<Args>(args)...);
  }

  template<typename... Args>
  void error(Args&&... args) {
    if (logger_) logger_->error(std::forward<Args>(args)...);
  }

  template<typename... Args>
  void debug(Args&&... args) {
    if (logger_) logger_->debug(std::forward<Args>(args)...);
  }

  void log_raw(const std::string& message);
  bool is_enabled() const { return enabled_; }

private:
  LogManager() = default;

  std::shared_ptr<spdlog::logger> logger_;
  std::mutex mutex_;
  bool enabled_ = false;
};

} // namespace exile::core