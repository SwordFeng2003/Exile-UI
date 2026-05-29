#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <Windows.h>

namespace exile::ui {

struct RadialEntry {
  std::string label;
  std::string icon;
  std::function<void()> action;
  std::function<void()> long_press_action;
  int position = 0;
};

class RadialMenu {
public:
  void show(int x, int y, const std::vector<RadialEntry>& entries, bool animated = true);
  void show_submenu(int x, int y, const std::vector<RadialEntry>& entries);
  void hide();
  bool is_visible() const { return visible_; }

  void render();

  void set_callback(std::function<void(const std::string&)> cb) { selection_callback_ = std::move(cb); }
  void set_cancel_callback(std::function<void()> cb) { cancel_callback_ = std::move(cb); }

  static constexpr int kGridSize = 3;
  static constexpr int kCenterPos = 5;

private:
  int position_to_grid(int pos) const;
  ImVec2 grid_to_coords(int grid_pos, int cell_size, int margin) const;

  bool visible_ = false;
  int origin_x_ = 0;
  int origin_y_ = 0;
  int cell_size_ = 48;
  int margin_ = 4;
  bool animated_ = true;
  float anim_progress_ = 0.0f;

  std::vector<RadialEntry> entries_;
  int hovered_entry_ = -1;

  std::function<void(const std::string&)> selection_callback_;
  std::function<void()> cancel_callback_;
};

} // namespace exile::ui