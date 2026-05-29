#pragma once

#include <string>
#include <vector>
#include <functional>
#include <imgui.h>

namespace exile::ui {

ImVec2 compute_panel_dimensions(const std::vector<std::string>& items, float font_size,
                               const char* font_name = nullptr);

void draw_graph(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size,
               const std::vector<float>& data, ImU32 color, ImU32 bg_color = IM_COL32(0,0,0,255));

bool render_dropdown(const std::string& label, const std::vector<std::string>& items,
                    int& current_index, float width = 0.0f);

bool render_color_picker(const std::string& label, ImVec4& color);

void render_help_tooltip(const std::string& text, float max_width = 300.0f);

void render_progress_bar(const std::string& label, float fraction, const ImVec2& size = ImVec2(-1, 0));

void render_timed_tooltip(const std::string& text, float duration_seconds = 3.0f,
                         ImU32 color = IM_COL32(0, 255, 0, 255));

void render_overlay_button(const std::string& label, const ImVec2& pos, const ImVec2& size,
                          ImU32 color, ImU32 hover_color, std::function<void()> on_click);

bool render_context_menu(const std::string& name, const std::vector<std::pair<std::string, std::function<void()>>>& items);

void render_border_frame(const ImVec2& pos, const ImVec2& size, ImU32 color, float thickness = 1.0f);

bool is_mouse_over_rect(const ImVec2& rect_min, const ImVec2& rect_max);

void clamp_to_monitor(int& x, int& y, int w, int h, int monitor_w, int monitor_h);

float get_default_font_size();
void set_default_font_size(float size);

ImVec2 get_font_dimensions(float size, const char* text = "7");

} // namespace exile::ui