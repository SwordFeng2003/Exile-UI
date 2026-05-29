#include "ui/imgui_renderer.h"
#include "ui/theme.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3dcompiler.h>

namespace exile::ui {

ImGuiRenderer* g_renderer = nullptr;

bool ImGuiRenderer::initialize(HWND target_hwnd) {
  g_renderer = this;
  target_hwnd_ = target_hwnd;

  IMGUI_CHECKVERSION();
  imgui_context_ = ImGui::CreateContext();
  ImGui::SetCurrentContext(imgui_context_);

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.IniFilename = "ini/imgui.ini";

  apply_exile_ui_theme();

  if (!create_device()) {
    return false;
  }

  ImGui_ImplWin32_Init(target_hwnd);
  ImGui_ImplDX11_Init(d3d_device_, d3d_context_);

  default_font_ = io.Fonts->AddFontDefault();
  io.FontDefault = default_font_;

  return true;
}

void ImGuiRenderer::shutdown() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext(imgui_context_);
  cleanup_device();
  g_renderer = nullptr;
}

bool ImGuiRenderer::create_device() {
  DXGI_SWAP_CHAIN_DESC sd = {};
  sd.BufferCount = 2;
  sd.BufferDesc.Width = window_width_;
  sd.BufferDesc.Height = window_height_;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = target_hwnd_;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  D3D_FEATURE_LEVEL feature_levels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
  };

  UINT create_device_flags = 0;
#ifdef _DEBUG
  create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  HRESULT hr = D3D11CreateDeviceAndSwapChain(
    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags,
    feature_levels, _countof(feature_levels), D3D11_SDK_VERSION,
    &sd, &swap_chain_, &d3d_device_, nullptr, &d3d_context_
  );

  if (FAILED(hr)) return false;

  create_render_target();
  return true;
}

void ImGuiRenderer::cleanup_device() {
  cleanup_render_target();
  if (swap_chain_) { swap_chain_->Release(); swap_chain_ = nullptr; }
  if (d3d_context_) { d3d_context_->Flush(); d3d_context_->Release(); d3d_context_ = nullptr; }
  if (d3d_device_) { d3d_device_->Release(); d3d_device_ = nullptr; }
}

void ImGuiRenderer::create_render_target() {
  ID3D11Texture2D* back_buffer = nullptr;
  swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
  if (back_buffer) {
    d3d_device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_);
    back_buffer->Release();
  }
}

void ImGuiRenderer::cleanup_render_target() {
  if (render_target_view_) { render_target_view_->Release(); render_target_view_ = nullptr; }
}

void ImGuiRenderer::resize(int width, int height) {
  window_width_ = width;
  window_height_ = height;

  cleanup_render_target();
  swap_chain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
  create_render_target();
}

void ImGuiRenderer::begin_frame() {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void ImGuiRenderer::end_frame() {
  ImGui::Render();

  const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  const float clear_color_rgba[4] = { clear_color.x, clear_color.y, clear_color.z, clear_color.w };
  d3d_context_->OMSetRenderTargets(1, &render_target_view_, nullptr);
  d3d_context_->ClearRenderTargetView(render_target_view_, clear_color_rgba);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  swap_chain_->Present(1, 0);
}

void ImGuiRenderer::render() {
  begin_frame();
  end_frame();
}

ImFont* ImGuiRenderer::load_font(const std::string& path, float size) {
  ImGuiIO& io = ImGui::GetIO();
  return io.Fonts->AddFontFromFileTTF(path.c_str(), size);
}

void ImGuiRenderer::set_default_font_size(float size) {
  font_size_ = size;
}

void ImGuiRenderer::set_overlay_transparency(float alpha) {
  ImGui::GetStyle().Alpha = alpha;
}

} // namespace exile::ui