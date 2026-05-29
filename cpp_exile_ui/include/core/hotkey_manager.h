#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <Windows.h>

namespace exile::core {

enum class HotkeyModifier : uint32_t {
  None = 0,
  Ctrl = MOD_CONTROL,
  Alt = MOD_ALT,
  Shift = MOD_SHIFT,
  Win = MOD_WIN,
};

struct HotkeyBinding {
  uint32_t vk_code;
  uint32_t modifiers;
  std::string name;
  std::function<void()> callback;
  bool enabled = true;
};

class HotkeyManager {
public:
  HotkeyManager();
  ~HotkeyManager();

  bool register_hotkey(const std::string& name, const std::string& key_expression,
                       std::function<void()> callback);
  bool register_hotkey_vk(const std::string& name, uint32_t vk_code, uint32_t modifiers,
                          std::function<void()> callback);
  void unregister_hotkey(const std::string& name);
  void unregister_all();

  void enable_hotkey(const std::string& name, bool enable = true);
  bool is_hotkey_enabled(const std::string& name) const;

  uint32_t vk_from_name(const std::string& key_name) const;
  uint32_t sc_to_vk(uint32_t scan_code) const;
  std::string convert_key(const std::string& key) const;

  void set_omnikey(const std::string& key) { omnikey_ = key; }
  void set_omnikey2(const std::string& key) { omnikey2_ = key; }
  void set_emergency_key(const std::string& key) { emergency_key_ = key; }
  void set_move_key(const std::string& key) { move_key_ = key; }
  void set_tab_key(const std::string& key) { tab_key_ = key; }
  void set_menu_widget_key(const std::string& key) { menu_widget_key_ = key; }

  const std::string& omnikey() const { return omnikey_; }
  const std::string& emergency_key() const { return emergency_key_; }

  void process_pending_callbacks();

private:
  static LRESULT CALLBACK keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam);
  static HotkeyManager* instance_;

  int next_id_ = 1;
  std::unordered_map<int, HotkeyBinding> bindings_;
  std::unordered_map<std::string, int> name_to_id_;
  std::vector<std::function<void()>> pending_callbacks_;
  std::mutex callback_mutex_;

  std::string omnikey_ = "capslock";
  std::string omnikey2_;
  std::string emergency_key_ = "space";
  std::string move_key_ = "lbutton";
  std::string tab_key_ = "tab";
  std::string menu_widget_key_;

  HHOOK keyboard_hook_ = nullptr;
  bool omnikey_pressed_ = false;
  bool omnikey2_pressed_ = false;
  uint64_t omnikey_press_time_ = 0;
};

} // namespace exile::core