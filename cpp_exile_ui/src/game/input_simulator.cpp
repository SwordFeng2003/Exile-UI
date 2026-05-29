#include "game/input_simulator.h"
#include <thread>

namespace exile::game {

int InputSimulator::key_delay_ms_ = 30;
HWND InputSimulator::target_window_ = nullptr;

void InputSimulator::send_key_down(uint32_t vk_code) {
  keybd_event(static_cast<BYTE>(vk_code), 0, 0, 0);
}

void InputSimulator::send_key_up(uint32_t vk_code) {
  keybd_event(static_cast<BYTE>(vk_code), 0, KEYEVENTF_KEYUP, 0);
}

void InputSimulator::send_key_press(uint32_t vk_code, int duration_ms) {
  send_key_down(vk_code);
  if (duration_ms > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
  }
  send_key_up(vk_code);
}

void InputSimulator::send_key_combo(const std::vector<uint32_t>& vk_codes) {
  for (auto vk : vk_codes) {
    send_key_down(vk);
    std::this_thread::sleep_for(std::chrono::milliseconds(key_delay_ms_));
  }
  for (auto it = vk_codes.rbegin(); it != vk_codes.rend(); ++it) {
    send_key_up(*it);
    std::this_thread::sleep_for(std::chrono::milliseconds(key_delay_ms_));
  }
}

void InputSimulator::send_text(const std::string& text) {
  for (char c : text) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = VkKeyScan(c);
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(key_delay_ms_));
  }
}

void InputSimulator::send_ctrl_c() {
  send_key_down(VK_CONTROL);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  send_key_press('C', 10);
  send_key_up(VK_CONTROL);
}

void InputSimulator::send_ctrl_v() {
  send_key_down(VK_CONTROL);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  send_key_press('V', 10);
  send_key_up(VK_CONTROL);
}

void InputSimulator::send_ctrl_a() {
  send_key_down(VK_CONTROL);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  send_key_press('A', 10);
  send_key_up(VK_CONTROL);
}

void InputSimulator::send_escape() {
  send_key_press(VK_ESCAPE, 20);
}

void InputSimulator::send_enter() {
  send_key_press(VK_RETURN, 20);
}

void InputSimulator::send_tab() {
  send_key_press(VK_TAB, 20);
}

void InputSimulator::send_space() {
  send_key_press(VK_SPACE, 20);
}

void InputSimulator::send_mouse_click(int x, int y, bool right_button) {
  SetCursorPos(x, y);

  DWORD down_flag = right_button ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
  DWORD up_flag = right_button ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;

  INPUT input = {};
  input.type = INPUT_MOUSE;
  input.mi.dx = x;
  input.mi.dy = y;
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
  SendInput(1, &input, sizeof(INPUT));

  input.mi.dwFlags = down_flag;
  SendInput(1, &input, sizeof(INPUT));

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  input.mi.dwFlags = up_flag;
  SendInput(1, &input, sizeof(INPUT));
}

void InputSimulator::send_mouse_move(int x, int y) {
  SetCursorPos(x, y);
}

void InputSimulator::send_mouse_wheel(int delta) {
  INPUT input = {};
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_WHEEL;
  input.mi.mouseData = delta * WHEEL_DELTA;
  SendInput(1, &input, sizeof(INPUT));
}

bool InputSimulator::is_key_down(uint32_t vk_code) {
  return (GetAsyncKeyState(vk_code) & 0x8000) != 0;
}

void InputSimulator::get_mouse_pos(int& x, int& y) {
  POINT pt;
  GetCursorPos(&pt);
  x = pt.x;
  y = pt.y;
}

} // namespace exile::game