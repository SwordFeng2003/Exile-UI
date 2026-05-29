#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace exile::modules {

class QolTools {
public:
  void initialize();
  void shutdown();

  void render_panel();
  void on_omnikey(const std::string& key);

  void set_blight_auto_tower(bool v) { blight_auto_tower_ = v; }
  void set_auto_flask(bool v) { auto_flask_ = v; }
  void set_auto_level_gem(bool v) { auto_level_gem_ = v; }
  void set_auto_quality_gem(bool v) { auto_quality_gem_ = v; }
  void set_auto_id(bool v) { auto_id_ = v; }

  bool auto_flask() const { return auto_flask_; }
  bool auto_level_gem() const { return auto_level_gem_; }

  void check_life();
  void check_es();
  void check_mana();
  void check_flask_slots();

  void set_flask_threshold(int n, float threshold);
  void set_life_threshold(float threshold);
  void set_es_threshold(float threshold);
  void set_mana_threshold(float threshold);

  void read_chat_commands();
  void process_commands(const std::string& line);
  void execute_command(const std::string& cmd, const std::string& args);

  void block_keyboard(bool block) { keyboard_blocked_ = block; }
  void block_mouse(bool block) { mouse_blocked_ = block; }

  bool is_boss_toggle() const { return boss_toggle_; }
  void set_boss_toggle(bool v) { boss_toggle_ = v; }

private:
  void send_flask(int slot);

  bool blight_auto_tower_ = false;
  bool auto_flask_ = false;
  bool auto_level_gem_ = false;
  bool auto_quality_gem_ = false;
  bool auto_id_ = false;
  bool boss_toggle_ = false;
  bool keyboard_blocked_ = false;
  bool mouse_blocked_ = false;
  float life_threshold_ = 0.5f;
  float es_threshold_ = 0.5f;
  float mana_threshold_ = 0.3f;
  std::vector<float> flask_thresholds_ = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
};

} // namespace exile::modules