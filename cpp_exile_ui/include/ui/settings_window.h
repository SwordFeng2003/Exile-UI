#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

struct ImVec2;

namespace exile::ui {

struct SettingsTab {
  std::string name;
  std::string label;
  std::function<void()> render_func;
  bool is_beta = false;
};

class SettingsWindow {
public:
  void register_tab(const SettingsTab& tab);
  void show();
  void hide();
  bool is_visible() const { return visible_; }

  void set_active_tab(const std::string& name);
  std::string active_tab() const { return active_tab_; }

  void render();
  void render_general_settings();
  void render_feature_toggles();
  void render_hotkey_settings();
  void render_screen_check_settings();
  void render_about();

  int window_width() const { return window_width_; }
  int selection_width() const { return selection_width_; }
  int window_x() const { return window_x_; }
  int window_y() const { return window_y_; }

private:
  void render_tab_bar();
  void render_tab_content();

  std::vector<SettingsTab> tabs_;
  std::string active_tab_ = "general";
  bool visible_ = false;

  int window_width_ = 800;
  int selection_width_ = 200;
  int window_x_ = 100;
  int window_y_ = 100;
};

} // namespace exile::ui