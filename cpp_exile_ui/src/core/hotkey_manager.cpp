#include "core/hotkey_manager.h"
#include <algorithm>

namespace exile::core {

HotkeyManager* HotkeyManager::instance_ = nullptr;

HotkeyManager::HotkeyManager() {
  instance_ = this;
}

HotkeyManager::~HotkeyManager() {
  unregister_all();
  instance_ = nullptr;
}

bool HotkeyManager::register_hotkey(const std::string& name, const std::string& key_expression,
                                     std::function<void()> callback) {
  uint32_t modifiers = 0;
  std::string key_part = key_expression;

  auto has_prefix = [&](const std::string& prefix) -> bool {
    auto upper = key_part;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    if (upper.find(prefix + "+") == 0) {
      key_part = key_part.substr(prefix.size() + 1);
      return true;
    }
    return false;
  };

  while (true) {
    bool found = false;
    if (has_prefix("CTRL") || has_prefix("^")) { modifiers |= MOD_CONTROL; found = true; }
    if (has_prefix("ALT") || has_prefix("!")) { modifiers |= MOD_ALT; found = true; }
    if (has_prefix("SHIFT") || has_prefix("+")) { modifiers |= MOD_SHIFT; found = true; }
    if (has_prefix("WIN") || has_prefix("#")) { modifiers |= MOD_WIN; found = true; }
    if (!found) break;
  }

  uint32_t vk = vk_from_name(key_part);
  if (vk == 0) return false;

  return register_hotkey_vk(name, vk, modifiers, std::move(callback));
}

bool HotkeyManager::register_hotkey_vk(const std::string& name, uint32_t vk_code,
                                        uint32_t modifiers, std::function<void()> callback) {
  int id = next_id_++;

  HotkeyBinding binding;
  binding.vk_code = vk_code;
  binding.modifiers = modifiers;
  binding.name = name;
  binding.callback = std::move(callback);
  binding.enabled = true;

  bindings_[id] = std::move(binding);
  name_to_id_[name] = id;

  return RegisterHotKey(nullptr, id, modifiers, vk_code);
}

void HotkeyManager::unregister_hotkey(const std::string& name) {
  auto it = name_to_id_.find(name);
  if (it != name_to_id_.end()) {
    UnregisterHotKey(nullptr, it->second);
    bindings_.erase(it->second);
    name_to_id_.erase(it);
  }
}

void HotkeyManager::unregister_all() {
  for (auto& [id, _] : bindings_) {
    UnregisterHotKey(nullptr, id);
  }
  bindings_.clear();
  name_to_id_.clear();
}

void HotkeyManager::enable_hotkey(const std::string& name, bool enable) {
  auto it = name_to_id_.find(name);
  if (it != name_to_id_.end()) {
    bindings_[it->second].enabled = enable;
  }
}

bool HotkeyManager::is_hotkey_enabled(const std::string& name) const {
  auto it = name_to_id_.find(name);
  if (it != name_to_id_.end()) {
    auto bit = bindings_.find(it->second);
    if (bit != bindings_.end()) return bit->second.enabled;
  }
  return false;
}

uint32_t HotkeyManager::vk_from_name(const std::string& key_name) const {
  static const std::unordered_map<std::string, uint32_t> key_map = {
    {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
    {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
    {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},
    {"SPACE", VK_SPACE}, {"TAB", VK_TAB}, {"ENTER", VK_RETURN},
    {"ESC", VK_ESCAPE}, {"ESCAPE", VK_ESCAPE},
    {"CAPSLOCK", VK_CAPITAL}, {"CAPS", VK_CAPITAL},
    {"LSHIFT", VK_LSHIFT}, {"RSHIFT", VK_RSHIFT},
    {"LCTRL", VK_LCONTROL}, {"RCTRL", VK_RCONTROL},
    {"LALT", VK_LMENU}, {"RALT", VK_RMENU},
    {"LWIN", VK_LWIN}, {"RWIN", VK_RWIN},
    {"BACKSPACE", VK_BACK}, {"BS", VK_BACK},
    {"DELETE", VK_DELETE}, {"DEL", VK_DELETE},
    {"INSERT", VK_INSERT}, {"INS", VK_INSERT},
    {"HOME", VK_HOME}, {"END", VK_END},
    {"PGUP", VK_PRIOR}, {"PGDN", VK_NEXT},
    {"UP", VK_UP}, {"DOWN", VK_DOWN},
    {"LEFT", VK_LEFT}, {"RIGHT", VK_RIGHT},
    {"PRINTSCREEN", VK_SNAPSHOT},
    {"SCROLLLOCK", VK_SCROLL},
    {"PAUSE", VK_PAUSE},
    {"NUMLOCK", VK_NUMLOCK},
    {"APPSKEY", VK_APPS},
    {"LBUTTON", VK_LBUTTON}, {"RBUTTON", VK_RBUTTON},
    {"MBUTTON", VK_MBUTTON},
    {"XBUTTON1", VK_XBUTTON1}, {"XBUTTON2", VK_XBUTTON2},
    {"WHEELUP", VK_UP}, {"WHEELDOWN", VK_DOWN},
    {"NUMPAD0", VK_NUMPAD0}, {"NUMPAD1", VK_NUMPAD1}, {"NUMPAD2", VK_NUMPAD2},
    {"NUMPAD3", VK_NUMPAD3}, {"NUMPAD4", VK_NUMPAD4}, {"NUMPAD5", VK_NUMPAD5},
    {"NUMPAD6", VK_NUMPAD6}, {"NUMPAD7", VK_NUMPAD7}, {"NUMPAD8", VK_NUMPAD8},
    {"NUMPAD9", VK_NUMPAD9},
    {"NUMPADADD", VK_ADD}, {"NUMPADSUB", VK_SUBTRACT},
    {"NUMPADMULT", VK_MULTIPLY}, {"NUMPADDIV", VK_DIVIDE},
    {"NUMPADDOT", VK_DECIMAL}, {"NUMPADENTER", VK_RETURN},
  };

  for (size_t i = 0; i <= 9; ++i) {
    if (key_name == std::to_string(i)) return 0x30 + i;
  }
  for (char c = 'A'; c <= 'Z'; ++c) {
    if (key_name == std::string(1, c)) return c;
  }

  auto it = key_map.find(key_name);
  if (it != key_map.end()) return it->second;

  auto upper = key_name;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  it = key_map.find(upper);
  if (it != key_map.end()) return it->second;

  return 0;
}

uint32_t HotkeyManager::sc_to_vk(uint32_t scan_code) const {
  return MapVirtualKey(scan_code, MAPVK_VSC_TO_VK);
}

std::string HotkeyManager::convert_key(const std::string& key) const {
  return key;
}

void HotkeyManager::process_pending_callbacks() {
  std::vector<std::function<void()>> callbacks;
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks.swap(pending_callbacks_);
  }
  for (auto& cb : callbacks) {
    if (cb) cb();
  }
}

} // namespace exile::core