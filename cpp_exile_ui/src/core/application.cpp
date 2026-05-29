#include "core/application.h"
#include "core/config_manager.h"
#include "core/log_manager.h"
#include "core/language_manager.h"
#include "core/thread_manager.h"
#include "core/hotkey_manager.h"
#include "core/overlay_manager.h"
#include "core/resource_manager.h"
#include "ui/imgui_renderer.h"

#include <chrono>
#include <thread>
#include <Windows.h>

namespace exile::core {

Application& Application::instance() {
  static Application app;
  return app;
}

Application::~Application() {
  shutdown();
}

bool Application::initialize(const ApplicationConfig& config) {
  config_ = config;

  config_mgr_ = std::make_unique<ConfigManager>();
  log_mgr_ = std::make_unique<LogManager>();
  lang_mgr_ = std::make_unique<LanguageManager>();
  thread_mgr_ = std::make_unique<ThreadManager>();
  hotkey_mgr_ = std::make_unique<HotkeyManager>();
  overlay_mgr_ = std::make_unique<OverlayManager>();
  resource_mgr_ = std::make_unique<ResourceManager>();

  resource_mgr_->set_base_path("data");

  config_mgr_->load("ini/config.ini");
  lang_mgr_->load_ui_language("data/languages/ui-english.txt");

  auto lang = config_mgr_->read<std::string>("English", "active-ui-language", "english");
  if (lang != "english") {
    lang_mgr_->load_ui_language("data/languages/ui-" + lang + ".txt");
    lang_mgr_->set_ui_lang(lang);
  }

  auto client_lang = config_mgr_->read<std::string>("English", "active-client-language", "english");
  lang_mgr_->load_client_language("data/languages/client-english.txt");
  if (client_lang != "english") {
    lang_mgr_->load_client_language("data/languages/client-" + client_lang + ".txt");
    lang_mgr_->set_client_lang(client_lang);
  }

  thread_mgr_->start_secondary_thread();

  return true;
}

void Application::run() {
  running_ = true;

  auto last_time = std::chrono::high_resolution_clock::now();

  while (running_) {
    auto now = std::chrono::high_resolution_clock::now();
    delta_time_ = std::chrono::duration<float>(now - last_time).count();
    last_time = now;

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        running_ = false;
      }
    }

    on_frame();

    static auto last_tick = now;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick).count() >= 1000) {
      on_second_tick();
      last_tick = now;
    }

    thread_mgr_->process_main_queue();

    if (!config_.dev_env) {
      auto frame_duration = std::chrono::milliseconds(1000 / config_.target_fps);
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::high_resolution_clock::now() - now);
      if (elapsed < frame_duration) {
        std::this_thread::sleep_for(frame_duration - elapsed);
      }
    }
  }
}

void Application::shutdown() {
  if (running_) {
    running_ = false;
  }
  thread_mgr_->stop_secondary_thread();
}

void Application::on_frame() {
  hotkey_mgr_->process_pending_callbacks();
}

void Application::on_second_tick() {
}

} // namespace exile::core