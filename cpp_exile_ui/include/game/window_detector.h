#pragma once

#include <string>
#include <Windows.h>

namespace exile::game {

struct ClientInfo {
  HWND hwnd = nullptr;
  std::string window_class;

  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
  int original_x = 0;
  int original_y = 0;
  int original_width = 0;
  int original_height = 0;
  int center_x = 0;
  int center_y = 0;

  bool is_fullscreen = true;
  bool is_borderless = true;
  bool is_streaming = false;
  bool is_docked_center = true;
  bool is_docked_top = true;

  std::string poe_version; // "" for PoE1, " 2" for PoE2
  int x_offset = 0;
  int y_offset = 0;
  bool closed = false;
};

struct MonitorInfo {
  int x = 0;
  int y = 0;
  int width = 1920;
  int height = 1080;
  int center_x = 960;
  int center_y = 540;
};

class WindowDetector {
public:
  bool detect_game_window();
  bool wait_for_game_window(int timeout_minutes = 0);

  const ClientInfo& client() const { return client_; }
  const MonitorInfo& monitor() const { return monitor_; }

  std::string detect_poe_version() const;
  std::string find_client_config_path() const;
  std::string find_log_file_path() const;
  std::string find_config_folder() const;

  bool read_client_config(const std::string& config_path);
  void apply_custom_resolution(int width, int height);
  void apply_window_position();
  void apply_borderless_toggle();

  bool is_game_active() const;
  bool is_game_focused() const;
  void focus_game_window();

  void update_client_position();

private:
  ClientInfo client_;
  MonitorInfo monitor_;
  RECT monitor_rect_ = {0, 0, 1920, 1080};
};

} // namespace exile::game