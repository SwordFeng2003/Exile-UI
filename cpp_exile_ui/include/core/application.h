#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace exile::core {

class ConfigManager;
class LogManager;
class LanguageManager;
class ThreadManager;
class HotkeyManager;
class OverlayManager;
class ResourceManager;

struct ApplicationConfig {
  std::string data_dir = "data";
  std::string ini_dir = "ini";
  std::string img_dir = "img";
  bool dev_mode = false;
  bool dev_env = false;
  int target_fps = 60;
  int script_kill_timeout = 60;
  bool kill_script = true;
};

class Application {
public:
  static Application& instance();

  bool initialize(const ApplicationConfig& config = {});
  void run();
  void shutdown();

  ConfigManager& config_manager() { return *config_mgr_; }
  LogManager& log_manager() { return *log_mgr_; }
  LanguageManager& language_manager() { return *lang_mgr_; }
  ThreadManager& thread_manager() { return *thread_mgr_; }
  HotkeyManager& hotkey_manager() { return *hotkey_mgr_; }
  OverlayManager& overlay_manager() { return *overlay_mgr_; }
  ResourceManager& resource_manager() { return *resource_mgr_; }

  const ApplicationConfig& config() const { return config_; }
  bool is_running() const { return running_; }
  void request_shutdown() { running_ = false; }

private:
  Application() = default;
  ~Application();
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;

  void on_frame();
  void on_second_tick();

  ApplicationConfig config_;
  bool running_ = false;

  std::unique_ptr<ConfigManager> config_mgr_;
  std::unique_ptr<LogManager> log_mgr_;
  std::unique_ptr<LanguageManager> lang_mgr_;
  std::unique_ptr<ThreadManager> thread_mgr_;
  std::unique_ptr<HotkeyManager> hotkey_mgr_;
  std::unique_ptr<OverlayManager> overlay_mgr_;
  std::unique_ptr<ResourceManager> resource_mgr_;

  uint64_t last_frame_time_ = 0;
  uint64_t last_second_tick_ = 0;
  float delta_time_ = 0.0f;
};

} // namespace exile::core