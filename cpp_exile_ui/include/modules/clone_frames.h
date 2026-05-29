#pragma once

#include <vector>
#include <string>
#include <Windows.h>

namespace exile::modules {

struct CloneFrame {
  std::string name;
  int source_x = 0;
  int source_y = 0;
  int source_w = 0;
  int source_h = 0;
  int target_x = 0;
  int target_y = 0;
  int target_w = 0;
  int target_h = 0;
  float scale_x = 1.0f;
  float scale_y = 1.0f;
  float opacity = 1.0f;
  bool enabled = true;
  bool mirror = false;
  HWND overlay_hwnd = nullptr;
};

class CloneFrames {
public:
  void initialize();
  void shutdown();

  void add_frame(const CloneFrame& frame);
  void remove_frame(size_t index);
  void update_frame(size_t index, const CloneFrame& frame);

  void start_editing();
  void stop_editing(bool save = true);
  bool is_editing() const { return editing_; }

  void render_all();
  void render_borders();
  void capture_frames();
  void hide_all();
  void show_all();

  const std::vector<CloneFrame>& frames() const { return frames_; }
  size_t frame_count() const { return frames_.size(); }

  void set_gamescreen_check(bool val) { gamescreen_check_ = val; }
  void set_closebutton_check(bool val) { closebutton_check_ = val; }
  void set_inventory_check(bool val) { inventory_check_ = val; }

private:
  std::vector<CloneFrame> frames_;
  bool editing_ = false;
  bool gamescreen_check_ = true;
  bool closebutton_check_ = false;
  bool inventory_check_ = false;
  float target_fps_ = 30.0f;
  int frame_counter_ = 0;
};

} // namespace exile::modules