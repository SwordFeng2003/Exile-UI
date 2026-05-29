#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace exile::game {

class InputSimulator {
public:
  static void send_key_down(uint32_t vk_code);
  static void send_key_up(uint32_t vk_code);
  static void send_key_press(uint32_t vk_code, int duration_ms = 50);
  static void send_key_combo(const std::vector<uint32_t>& vk_codes);

  static void send_text(const std::string& text);
  static void send_ctrl_c();
  static void send_ctrl_v();
  static void send_ctrl_a();
  static void send_escape();
  static void send_enter();
  static void send_tab();
  static void send_space();

  static void send_mouse_click(int x, int y, bool right_button = false);
  static void send_mouse_move(int x, int y);
  static void send_mouse_wheel(int delta);

  static void set_key_delay(int ms) { key_delay_ms_ = ms; }
  static int key_delay() { return key_delay_ms_; }

  static void set_window(HWND hwnd) { target_window_ = hwnd; }
  static HWND target_window() { return target_window_; }

  static bool is_key_down(uint32_t vk_code);
  static void get_mouse_pos(int& x, int& y);

private:
  static int key_delay_ms_;
  static HWND target_window_;
};

} // namespace exile::game