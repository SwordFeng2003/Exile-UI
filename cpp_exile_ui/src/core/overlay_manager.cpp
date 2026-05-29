#include "core/overlay_manager.h"
#include <algorithm>

namespace exile::core {

static const wchar_t* kOverlayClass = L"ExileUI_Overlay";

OverlayManager::OverlayManager() {
  WNDCLASSEXW wc = {};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = overlay_wnd_proc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = kOverlayClass;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  RegisterClassExW(&wc);
}

OverlayManager::~OverlayManager() {
  destroy_client_filler();
  for (auto& overlay : overlays_) {
    if (overlay.hwnd) DestroyWindow(overlay.hwnd);
  }
  overlays_.clear();
}

HWND OverlayManager::create_overlay(const std::string& name, int x, int y, int w, int h,
                                     bool click_through, bool always_on_top) {
  auto* found = find_overlay(name);
  if (found) return found->hwnd;

  DWORD ex_style = WS_EX_TOOLWINDOW | WS_EX_LAYERED;
  if (click_through) ex_style |= WS_EX_TRANSPARENT;
  if (always_on_top) ex_style |= WS_EX_TOPMOST;

  HWND hwnd = CreateWindowExW(
    ex_style, kOverlayClass, L"",
    WS_POPUP,
    x, y, w, h,
    nullptr, nullptr, GetModuleHandle(nullptr), nullptr
  );

  if (!hwnd) return nullptr;

  SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

  OverlayWindow overlay;
  overlay.hwnd = hwnd;
  overlay.name = name;
  overlay.visible = false;
  overlay.x = x;
  overlay.y = y;
  overlay.width = w;
  overlay.height = h;
  overlay.transparency = 255;
  overlay.click_through = click_through;
  overlay.always_on_top = always_on_top;

  overlays_.push_back(overlay);
  return hwnd;
}

void OverlayManager::destroy_overlay(const std::string& name) {
  auto it = std::find_if(overlays_.begin(), overlays_.end(),
      [&](const OverlayWindow& o) { return o.name == name; });
  if (it != overlays_.end()) {
    if (it->hwnd) DestroyWindow(it->hwnd);
    overlays_.erase(it);
  }
}

void OverlayManager::destroy_overlay(HWND hwnd) {
  auto it = std::find_if(overlays_.begin(), overlays_.end(),
      [&](const OverlayWindow& o) { return o.hwnd == hwnd; });
  if (it != overlays_.end()) {
    DestroyWindow(it->hwnd);
    overlays_.erase(it);
  }
}

void OverlayManager::show_overlay(const std::string& name) {
  auto* overlay = find_overlay(name);
  if (overlay) {
    overlay->visible = true;
    ShowWindow(overlay->hwnd, SW_SHOWNOACTIVATE);
    SetWindowPos(overlay->hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }
}

void OverlayManager::hide_overlay(const std::string& name) {
  auto* overlay = find_overlay(name);
  if (overlay) {
    overlay->visible = false;
    ShowWindow(overlay->hwnd, SW_HIDE);
  }
}

void OverlayManager::hide_all() {
  for (auto& overlay : overlays_) {
    overlay.visible = false;
    ShowWindow(overlay.hwnd, SW_HIDE);
  }
}

void OverlayManager::show_all_visible() {
  for (auto& overlay : overlays_) {
    if (overlay.visible) {
      ShowWindow(overlay.hwnd, SW_SHOWNOACTIVATE);
    }
  }
}

void OverlayManager::set_position(const std::string& name, int x, int y, int w, int h) {
  auto* overlay = find_overlay(name);
  if (overlay) {
    overlay->x = x;
    overlay->y = y;
    overlay->width = w;
    overlay->height = h;
    SetWindowPos(overlay->hwnd, nullptr, x, y, w, h,
                 SWP_NOZORDER | SWP_NOACTIVATE);
  }
}

void OverlayManager::set_transparency(const std::string& name, uint8_t alpha) {
  auto* overlay = find_overlay(name);
  if (overlay) {
    overlay->transparency = alpha;
    SetLayeredWindowAttributes(overlay->hwnd, 0, alpha, LWA_ALPHA);
  }
}

void OverlayManager::set_click_through(const std::string& name, bool click_through) {
  auto* overlay = find_overlay(name);
  if (overlay) {
    overlay->click_through = click_through;
    LONG ex_style = GetWindowLong(overlay->hwnd, GWL_EXSTYLE);
    if (click_through) {
      ex_style |= WS_EX_TRANSPARENT;
    } else {
      ex_style &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLong(overlay->hwnd, GWL_EXSTYLE, ex_style);
  }
}

OverlayWindow* OverlayManager::find_overlay(const std::string& name) {
  auto it = std::find_if(overlays_.begin(), overlays_.end(),
      [&](const OverlayWindow& o) { return o.name == name; });
  if (it != overlays_.end()) return &(*it);
  return nullptr;
}

OverlayWindow* OverlayManager::find_overlay(HWND hwnd) {
  auto it = std::find_if(overlays_.begin(), overlays_.end(),
      [&](const OverlayWindow& o) { return o.hwnd == hwnd; });
  if (it != overlays_.end()) return &(*it);
  return nullptr;
}

void OverlayManager::check_bounds(int& x, int& y, int w, int h) const {
  if (x < monitor_rect_.left) x = monitor_rect_.left;
  if (y < monitor_rect_.top) y = monitor_rect_.top;
  if (x + w > monitor_rect_.right) x = monitor_rect_.right - w;
  if (y + h > monitor_rect_.bottom) y = monitor_rect_.bottom - h;
}

void OverlayManager::create_client_filler(int x, int y, int w, int h, bool cover_taskbar) {
  destroy_client_filler();

  HWND hwnd = CreateWindowExW(
    WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
    L"Static", L"", WS_POPUP | SS_BLACKFRAME,
    x, y, w, h, nullptr, nullptr, GetModuleHandle(nullptr), nullptr
  );

  if (hwnd) {
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    client_filler_hwnd_ = hwnd;
  }
}

void OverlayManager::destroy_client_filler() {
  if (client_filler_hwnd_) {
    DestroyWindow(client_filler_hwnd_);
    client_filler_hwnd_ = nullptr;
  }
}

LRESULT CALLBACK OverlayManager::overlay_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_DESTROY:
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_NCHITTEST:
      return HTTRANSPARENT;
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
}

} // namespace exile::core