#include "game/window_detector.h"
#include "core/config_manager.h"
#include "core/log_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace exile::game {

static BOOL CALLBACK enum_windows_callback(HWND hwnd, LPARAM lParam) {
  auto* client = reinterpret_cast<ClientInfo*>(lParam);
  wchar_t class_name[256] = {};
  GetClassNameW(hwnd, class_name, 256);

  std::wstring class_str(class_name);
  if (class_str.find(L"POEWindowClass") != std::wstring::npos ||
      class_str.find(L"DIRECT3D") != std::wstring::npos) {

    if (IsWindowVisible(hwnd) && !IsIconic(hwnd)) {
      RECT rect;
      GetWindowRect(hwnd, &rect);

      int w = rect.right - rect.left;
      int h = rect.bottom - rect.top;

      if (w > 800 && h > 600) {
        client->hwnd = hwnd;
        client->x = rect.left;
        client->y = rect.top;
        client->width = w;
        client->height = h;
        client->original_x = rect.left;
        client->original_y = rect.top;
        client->original_width = w;
        client->original_height = h;

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        client->window_class = converter.to_bytes(class_name);

        return FALSE;
      }
    }
  }
  return TRUE;
}

bool WindowDetector::detect_game_window() {
  client_.hwnd = nullptr;
  EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&client_));

  if (!client_.hwnd) return false;

  client_.center_x = client_.x + client_.width / 2;
  client_.center_y = client_.y + client_.height / 2;

  detect_poe_version();

  return true;
}

bool WindowDetector::wait_for_game_window(int timeout_minutes) {
  auto start = GetTickCount64();
  int timeout_ms = timeout_minutes > 0 ? timeout_minutes * 60000 : INFINITE;

  while (true) {
    if (detect_game_window()) return true;

    if (timeout_minutes > 0 &&
        GetTickCount64() - start > static_cast<uint64_t>(timeout_ms)) {
      return false;
    }

    Sleep(1000);
  }
}

std::string WindowDetector::detect_poe_version() const {
  auto log_path = find_log_file_path();
  if (log_path.empty()) return "";

  std::ifstream log(log_path);
  if (!log.is_open()) return "";

  std::string line;
  while (std::getline(log, line)) {
    if (line.find("Connecting to instance server") != std::string::npos) {
      if (line.find("Path of Exile 2") != std::string::npos) return " 2";
      return "";
    }
  }

  auto config_path = find_client_config_path();
  if (!config_path.empty()) {
    std::ifstream cfg(config_path);
    if (cfg.is_open()) {
      std::string cfg_line;
      while (std::getline(cfg, cfg_line)) {
        for (auto& c : cfg_line) c = std::tolower(c);
        if (cfg_line.find("poe2") != std::string::npos) return " 2";
      }
    }
  }

  return "";
}

std::string WindowDetector::find_client_config_path() const {
  wchar_t documents[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, documents))) {
    std::wstring path = std::wstring(documents) + L"\\My Games\\Path of Exile\\production_Config.ini";
    if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
      std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      return converter.to_bytes(path);
    }
    path = std::wstring(documents) + L"\\My Games\\Path of Exile 2\\production_Config.ini";
    if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
      std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      return converter.to_bytes(path);
    }
  }
  return "";
}

std::string WindowDetector::find_log_file_path() const {
  auto config_path = find_client_config_path();
  if (config_path.empty()) return "";

  std::filesystem::path cfg_path(config_path);
  auto game_dir = cfg_path.parent_path();
  return (game_dir / "logs" / "Client.txt").string();
}

std::string WindowDetector::find_config_folder() const {
  auto config_path = find_client_config_path();
  if (config_path.empty()) return "";
  return std::filesystem::path(config_path).parent_path().string();
}

bool WindowDetector::read_client_config(const std::string& config_path) {
  auto& cfg = core::Application::instance().config_manager();
  return cfg.load(config_path);
}

void WindowDetector::apply_custom_resolution(int width, int height) {
  client_.width = width;
  client_.height = height;
}

void WindowDetector::apply_window_position() {
  if (client_.hwnd) {
    SetWindowPos(client_.hwnd, nullptr, client_.x, client_.y,
                 client_.width, client_.height, SWP_NOZORDER);
  }
}

void WindowDetector::apply_borderless_toggle() {
  if (!client_.hwnd) return;

  LONG style = GetWindowLong(client_.hwnd, GWL_STYLE);
  LONG ex_style = GetWindowLong(client_.hwnd, GWL_EXSTYLE);

  if (client_.is_borderless) {
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    ex_style &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
  }

  SetWindowLong(client_.hwnd, GWL_STYLE, style);
  SetWindowLong(client_.hwnd, GWL_EXSTYLE, ex_style);
}

bool WindowDetector::is_game_active() const {
  HWND foreground = GetForegroundWindow();
  return foreground == client_.hwnd;
}

bool WindowDetector::is_game_focused() const {
  return GetForegroundWindow() == client_.hwnd;
}

void WindowDetector::focus_game_window() {
  if (client_.hwnd) {
    SetForegroundWindow(client_.hwnd);
  }
}

void WindowDetector::update_client_position() {
  if (client_.hwnd) {
    RECT rect;
    GetWindowRect(client_.hwnd, &rect);
    client_.x = rect.left;
    client_.y = rect.top;
    client_.width = rect.right - rect.left;
    client_.height = rect.bottom - rect.top;
    client_.center_x = client_.x + client_.width / 2;
    client_.center_y = client_.y + client_.height / 2;
  }
}

} // namespace exile::game