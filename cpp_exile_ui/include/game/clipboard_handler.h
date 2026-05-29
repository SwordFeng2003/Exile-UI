#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace exile::game {

class ClipboardHandler {
public:
  static ClipboardHandler& instance();

  std::string get_text();
  bool set_text(const std::string& text);
  bool has_text() const;
  void clear();

  bool copy_from_game(bool use_alt_modifier = false);
  bool wait_for_clipboard(int timeout_ms = 100);

  std::vector<std::string> get_lines() const;

private:
  ClipboardHandler() = default;
  std::string cached_text_;
};

} // namespace exile::game