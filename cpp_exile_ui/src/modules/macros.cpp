#include "modules/macros.h"
#include "core/config_manager.h"
#include "core/hotkey_manager.h"
#include "core/application.h"
#include "game/input_simulator.h"
#include <thread>

namespace exile::modules {

void Macros::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  send_delay_ms_ = cfg.read<int>("Macros", "send-delay", 1);
  mouse_delay_ms_ = cfg.read<int>("Macros", "mouse-delay", 1);

  load_macros(1);
}

void Macros::shutdown() {
  stop_all();
}

bool Macros::load_macros(int config_index) {
  auto& cfg = core::Application::instance().config_manager();
  auto prefix = "macro" + std::to_string(config_index);

  int count = cfg.read<int>("Macros", prefix + "-count", 0);
  macros_.clear();

  for (int i = 0; i < count; ++i) {
    Macro macro;
    auto mp = prefix + "-m" + std::to_string(i + 1);
    macro.name = cfg.read<std::string>("Macros", mp + "-name", "");
    macro.trigger_key = cfg.read<std::string>("Macros", mp + "-key", "");
    macro.enabled = cfg.read<bool>("Macros", mp + "-enabled", true);

    macros_.push_back(macro);
  }

  return true;
}

void Macros::save_macros(int config_index) {
  auto& cfg = core::Application::instance().config_manager();
  auto prefix = "macro" + std::to_string(config_index);

  cfg.write("Macros", prefix + "-count", static_cast<int>(macros_.size()));
  for (size_t i = 0; i < macros_.size(); ++i) {
    auto mp = prefix + "-m" + std::to_string(i + 1);
    cfg.write("Macros", mp + "-name", macros_[i].name);
    cfg.write("Macros", mp + "-key", macros_[i].trigger_key);
    cfg.write("Macros", mp + "-enabled", macros_[i].enabled);
  }
}

void Macros::run_macro(int index) {
  if (index < 0 || index >= static_cast<int>(macros_.size())) return;
  if (running_) return;

  auto& macro = macros_[index];
  if (!macro.enabled) return;

  running_ = true;
  stop_requested_ = false;

  int repeat = macro.repeat ? macro.repeat_count : 1;
  for (int r = 0; r < repeat && !stop_requested_; ++r) {
    execute_actions(macro.actions);
    if (r < repeat - 1 && !stop_requested_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(macro.repeat_delay));
    }
  }

  running_ = false;
}

void Macros::run_macro_by_name(const std::string& name) {
  for (size_t i = 0; i < macros_.size(); ++i) {
    if (macros_[i].name == name) {
      run_macro(static_cast<int>(i));
      return;
    }
  }
}

void Macros::stop_all() {
  stop_requested_ = true;
  running_ = false;
}

void Macros::add_macro(const Macro& macro) {
  macros_.push_back(macro);
}

void Macros::remove_macro(size_t index) {
  if (index < macros_.size()) {
    macros_.erase(macros_.begin() + index);
  }
}

void Macros::update_macro(size_t index, const Macro& macro) {
  if (index < macros_.size()) {
    macros_[index] = macro;
  }
}

void Macros::on_omnikey_trigger(const std::string& key) {
  for (auto& macro : macros_) {
    if (macro.trigger_key == key && macro.enabled) {
      run_macro_by_name(macro.name);
      return;
    }
  }
}

void Macros::execute_actions(const std::vector<MacroAction>& actions) {
  for (auto& action : actions) {
    if (stop_requested_) return;

    if (action.type == "send") {
      game::InputSimulator::send_text(action.param);
      std::this_thread::sleep_for(std::chrono::milliseconds(send_delay_ms_));
    } else if (action.type == "sleep") {
      int duration = action.param2 > 0 ? action.param2 : 100;
      std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    } else if (action.type == "mousemove") {
      int x = action.param2;
      int y = action.param3;
      game::InputSimulator::send_mouse_move(x, y);
      std::this_thread::sleep_for(std::chrono::milliseconds(mouse_delay_ms_));
    } else if (action.type == "mouseclick") {
      bool right = (action.param == "right");
      int x = action.param2;
      int y = action.param3;
      game::InputSimulator::send_mouse_click(x, y, right);
    } else if (action.type == "keydown") {
      auto vk = core::Application::instance().hotkey_manager().vk_from_name(action.param);
      game::InputSimulator::send_key_down(vk);
    } else if (action.type == "keyup") {
      auto vk = core::Application::instance().hotkey_manager().vk_from_name(action.param);
      game::InputSimulator::send_key_up(vk);
    }
  }
}

} // namespace exile::modules