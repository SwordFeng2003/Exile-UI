#include "core/log_manager.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace exile::core {

LogManager& LogManager::instance() {
  static LogManager mgr;
  return mgr;
}

void LogManager::initialize(const std::filesystem::path& log_path, bool enabled) {
  enabled_ = enabled;
  if (!enabled_) return;

  try {
    auto dir = log_path.parent_path();
    if (!std::filesystem::exists(dir)) {
      std::filesystem::create_directories(dir);
    }

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path.string(), true);

    std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};
    logger_ = std::make_shared<spdlog::logger>("exile_ui", sinks.begin(), sinks.end());
    logger_->set_level(spdlog::level::debug);
    logger_->flush_on(spdlog::level::info);

    spdlog::register_logger(logger_);
  } catch (const std::exception& e) {
    enabled_ = false;
  }
}

void LogManager::shutdown() {
  if (logger_) {
    logger_->flush();
    spdlog::drop("exile_ui");
    logger_.reset();
  }
  enabled_ = false;
}

void LogManager::log_raw(const std::string& message) {
  if (logger_ && enabled_) logger_->info(message);
}

} // namespace exile::core