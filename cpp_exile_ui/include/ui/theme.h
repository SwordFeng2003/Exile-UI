#pragma once

#include <imgui.h>

namespace exile::ui {

struct ThemeColors {
  ImVec4 background = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
  ImVec4 surface = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
  ImVec4 surface_hover = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
  ImVec4 border = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  ImVec4 accent = ImVec4(0.00f, 0.80f, 0.00f, 1.00f);
  ImVec4 accent_hover = ImVec4(0.20f, 1.00f, 0.20f, 1.00f);
  ImVec4 warning = ImVec4(1.00f, 0.50f, 0.00f, 1.00f);
  ImVec4 error = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
  ImVec4 text_primary = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  ImVec4 text_secondary = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
  ImVec4 text_dim = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  float rounding = 3.0f;
  float alpha = 1.0f;
};

void initialize_theme();
void apply_exile_ui_theme();
void set_theme_colors(const ThemeColors& colors);
const ThemeColors& get_theme_colors();

ImU32 to_im32(const ImVec4& color);
uint32_t hex_to_color(const std::string& hex);

} // namespace exile::ui