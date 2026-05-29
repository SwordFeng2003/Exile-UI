#include "ui/theme.h"
#include <imgui.h>

namespace exile::ui {

static ThemeColors g_theme_colors;

void initialize_theme() {
  g_theme_colors = ThemeColors{};
}

void apply_exile_ui_theme() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImGui::StyleColorsDark();

  style.WindowRounding = g_theme_colors.rounding;
  style.FrameRounding = g_theme_colors.rounding;
  style.ChildRounding = g_theme_colors.rounding;
  style.GrabRounding = g_theme_colors.rounding;
  style.PopupRounding = g_theme_colors.rounding;
  style.ScrollbarRounding = g_theme_colors.rounding;
  style.TabRounding = g_theme_colors.rounding;

  style.Alpha = g_theme_colors.alpha;

  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;

  ImVec4* colors = style.Colors;

  colors[ImGuiCol_WindowBg] = g_theme_colors.background;
  colors[ImGuiCol_ChildBg] = g_theme_colors.surface;
  colors[ImGuiCol_PopupBg] = g_theme_colors.surface;
  colors[ImGuiCol_FrameBg] = g_theme_colors.surface;
  colors[ImGuiCol_FrameBgHovered] = g_theme_colors.surface_hover;
  colors[ImGuiCol_FrameBgActive] = g_theme_colors.surface_hover;

  colors[ImGuiCol_TitleBg] = g_theme_colors.background;
  colors[ImGuiCol_TitleBgActive] = g_theme_colors.surface;
  colors[ImGuiCol_TitleBgCollapsed] = g_theme_colors.background;

  colors[ImGuiCol_Border] = g_theme_colors.border;
  colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

  colors[ImGuiCol_Button] = g_theme_colors.accent;
  colors[ImGuiCol_ButtonHovered] = g_theme_colors.accent_hover;
  colors[ImGuiCol_ButtonActive] = g_theme_colors.accent;

  colors[ImGuiCol_Header] = g_theme_colors.surface_hover;
  colors[ImGuiCol_HeaderHovered] = g_theme_colors.accent;
  colors[ImGuiCol_HeaderActive] = g_theme_colors.accent;

  colors[ImGuiCol_Tab] = g_theme_colors.surface;
  colors[ImGuiCol_TabHovered] = g_theme_colors.accent;
  colors[ImGuiCol_TabActive] = g_theme_colors.accent;

  colors[ImGuiCol_Text] = g_theme_colors.text_primary;
  colors[ImGuiCol_TextDisabled] = g_theme_colors.text_dim;

  colors[ImGuiCol_CheckMark] = g_theme_colors.accent;
  colors[ImGuiCol_SliderGrab] = g_theme_colors.accent;
  colors[ImGuiCol_SliderGrabActive] = g_theme_colors.accent_hover;

  colors[ImGuiCol_ScrollbarBg] = g_theme_colors.surface;
  colors[ImGuiCol_ScrollbarGrab] = g_theme_colors.border;
  colors[ImGuiCol_ScrollbarGrabHovered] = g_theme_colors.surface_hover;
  colors[ImGuiCol_ScrollbarGrabActive] = g_theme_colors.accent;

  colors[ImGuiCol_TableHeaderBg] = g_theme_colors.surface;
  colors[ImGuiCol_TableBorderStrong] = g_theme_colors.border;
  colors[ImGuiCol_TableBorderLight] = g_theme_colors.border;
}

void set_theme_colors(const ThemeColors& colors) {
  g_theme_colors = colors;
  apply_exile_ui_theme();
}

const ThemeColors& get_theme_colors() {
  return g_theme_colors;
}

ImU32 to_im32(const ImVec4& color) {
  return IM_COL32(
    static_cast<int>(color.x * 255),
    static_cast<int>(color.y * 255),
    static_cast<int>(color.z * 255),
    static_cast<int>(color.w * 255)
  );
}

uint32_t hex_to_color(const std::string& hex) {
  std::string h = hex;
  if (h.size() >= 1 && h[0] == '#') h = h.substr(1);
  if (h.size() < 6) return 0;

  uint32_t r = std::stoul(h.substr(0, 2), nullptr, 16);
  uint32_t g = std::stoul(h.substr(2, 2), nullptr, 16);
  uint32_t b = std::stoul(h.substr(4, 2), nullptr, 16);

  return (r << 16) | (g << 8) | b;
}

} // namespace exile::ui