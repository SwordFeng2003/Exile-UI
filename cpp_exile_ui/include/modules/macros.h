#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <Windows.h>

namespace exile::modules {

struct MacroAction {
  std::string type; // "send", "sleep", "mousemove", "mouseclick", "keydown", "keyup"
  std::string param;
  int param2 = 0;
  int param3 = 0;
};

struct Macro {
  std::string name;
  std::string trigger_key;
  bool enabled = true;
  bool repeat = false;
  int repeat_count = 1;
  int repeat_delay = 100;
  std::vector<MacroAction> actions;
};

class Macros {
public:
  void initialize();
  void shutdown();

  bool load_macros(int config_index);
  void save_macros(int config_index);

  void run_macro(int index);
  void run_macro_by_name(const std::string& name);
  void stop_all();

  void add_macro(const Macro& macro);
  void remove_macro(size_t index);
  void update_macro(size_t index, const Macro& macro);

  const std::vector<Macro>& macros() const { return macros_; }
  bool is_running() const { return running_; }

  void set_send_delay(int ms) { send_delay_ms_ = ms; }
  void set_mouse_delay(int ms) { mouse_delay_ms_ = ms; }

  void on_omnikey_trigger(const std::string& key);

private:
  void execute_actions(const std::vector<MacroAction>& actions);

  std::vector<Macro> macros_;
  bool running_ = false;
  std::atomic<bool> stop_requested_{false};
  int send_delay_ms_ = 1;
  int mouse_delay_ms_ = 1;
};

} // namespace exile::modules