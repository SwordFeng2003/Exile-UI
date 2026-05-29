#pragma once

#include <Windows.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <d3d11.h>

namespace exile::core {

struct OverlayWindow {
  HWND hwnd = nullptr;
  std::string name;
  bool visible = false;
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
  uint8_t transparency = 255;
  bool click_through = true;
  bool always_on_top = true;
};

class OverlayManager {
public:
  OverlayManager();
  ~OverlayManager();

  HWND create_overlay(const std::string& name, int x, int y, int w, int h,
                      bool click_through = true, bool always_on_top = true);
  void destroy_overlay(const std::string& name);
  void destroy_overlay(HWND hwnd);
  void show_overlay(const std::string& name);
  void hide_overlay(const std::string& name);
  void hide_all();
  void show_all_visible();

  void set_position(const std::string& name, int x, int y, int w, int h);
  void set_transparency(const std::string& name, uint8_t alpha);
  void set_click_through(const std::string& name, bool click_through);

  OverlayWindow* find_overlay(const std::string& name);
  OverlayWindow* find_overlay(HWND hwnd);
  const std::vector<OverlayWindow>& overlays() const { return overlays_; }

  void check_bounds(int& x, int& y, int w, int h) const;
  void set_monitor_rect(const RECT& rect) { monitor_rect_ = rect; }
  RECT monitor_rect() const { return monitor_rect_; }

  int font_size() const { return font_size_; }
  void set_font_size(int size) { font_size_ = size; }

  HWND client_filler_hwnd() const { return client_filler_hwnd_; }
  void create_client_filler(int x, int y, int w, int h, bool cover_taskbar = false);
  void destroy_client_filler();

private:
  static LRESULT CALLBACK overlay_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  std::vector<OverlayWindow> overlays_;
  RECT monitor_rect_ = {0, 0, 1920, 1080};
  int font_size_ = 14;
  HWND client_filler_hwnd_ = nullptr;
};

} // namespace exile::core