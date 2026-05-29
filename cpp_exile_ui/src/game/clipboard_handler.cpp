#include "game/clipboard_handler.h"
#include <algorithm>

namespace exile::game {

ClipboardHandler& ClipboardHandler::instance() {
  static ClipboardHandler handler;
  return handler;
}

std::string ClipboardHandler::get_text() {
  if (!OpenClipboard(nullptr)) return "";

  std::string result;
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData) {
    char* text = static_cast<char*>(GlobalLock(hData));
    if (text) {
      result = text;
      GlobalUnlock(hData);
    }
  }

  CloseClipboard();
  cached_text_ = result;
  return result;
}

bool ClipboardHandler::set_text(const std::string& text) {
  if (!OpenClipboard(nullptr)) return false;
  EmptyClipboard();

  size_t size = text.size() + 1;
  HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
  if (!hGlobal) {
    CloseClipboard();
    return false;
  }

  memcpy(GlobalLock(hGlobal), text.c_str(), size);
  GlobalUnlock(hGlobal);

  SetClipboardData(CF_TEXT, hGlobal);
  CloseClipboard();
  cached_text_ = text;
  return true;
}

bool ClipboardHandler::has_text() const {
  return !cached_text_.empty();
}

void ClipboardHandler::clear() {
  cached_text_.clear();
}

bool ClipboardHandler::copy_from_game(bool use_alt_modifier) {
  clear();

  if (use_alt_modifier) {
    keybd_event(VK_MENU, 0, 0, 0);
    Sleep(10);
    keybd_event('C', 0, 0, 0);
    Sleep(10);
    keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
  } else {
    keybd_event(VK_CONTROL, 0, 0, 0);
    Sleep(10);
    keybd_event('C', 0, 0, 0);
    Sleep(10);
    keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
  }

  Sleep(50);
  get_text();
  return has_text();
}

bool ClipboardHandler::wait_for_clipboard(int timeout_ms) {
  auto start = GetTickCount64();
  while (GetTickCount64() - start < static_cast<uint64_t>(timeout_ms)) {
    auto text = get_text();
    if (!text.empty()) return true;
    Sleep(10);
  }
  return false;
}

std::vector<std::string> ClipboardHandler::get_lines() const {
  std::vector<std::string> lines;
  std::string line;
  for (char c : cached_text_) {
    if (c == '\n') {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      lines.push_back(line);
      line.clear();
    } else {
      line += c;
    }
  }
  if (!line.empty()) lines.push_back(line);
  return lines;
}

} // namespace exile::game