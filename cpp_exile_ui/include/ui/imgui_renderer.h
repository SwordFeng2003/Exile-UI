#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

struct ImGuiContext;
struct ImFont;

namespace exile::ui {

struct GuiState {
  bool settings_open = false;
  bool radial_menu_open = false;
  bool overlay_visible = true;
  bool dragging = false;
  bool inactive = false;
  bool gui_hidden = false;
  int mouse_x = 0;
  int mouse_y = 0;
  HWND mouse_window = nullptr;
  uint32_t mouse_control = 0;
  int click_type = 1;
};

class ImGuiRenderer {
public:
  bool initialize(HWND target_hwnd);
  void shutdown();

  void begin_frame();
  void end_frame();
  void render();

  bool create_device();
  void cleanup_device();
  void create_render_target();
  void cleanup_render_target();

  void resize(int width, int height);

  ImFont* load_font(const std::string& path, float size);
  ImFont* default_font() { return default_font_; }
  void set_default_font_size(float size);

  GuiState& gui_state() { return gui_state_; }

  void set_overlay_transparency(float alpha);

  ID3D11Device* device() { return d3d_device_; }
  ID3D11DeviceContext* context() { return d3d_context_; }

private:
  ID3D11Device* d3d_device_ = nullptr;
  ID3D11DeviceContext* d3d_context_ = nullptr;
  IDXGISwapChain* swap_chain_ = nullptr;
  ID3D11RenderTargetView* render_target_view_ = nullptr;
  HWND target_hwnd_ = nullptr;

  ImGuiContext* imgui_context_ = nullptr;
  ImFont* default_font_ = nullptr;
  float font_size_ = 14.0f;

  GuiState gui_state_;
  int window_width_ = 1920;
  int window_height_ = 1080;
};

extern ImGuiRenderer* g_renderer;

} // namespace exile::ui