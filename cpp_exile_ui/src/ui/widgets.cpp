#include "ui/widgets.h"
#include "ui/theme.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace exile::ui {

ImVec2 compute_panel_dimensions(const std::vector<std::string>& items, float font_size,
                                const char* font_name) {
  float max_width = 0.0f;
  float line_height = font_size * 1.5f;

  for (const auto& item : items) {
    float w = ImGui::CalcTextSize(item.c_str()).x;
    if (w > max_width) max_width = w;
  }

  return ImVec2(max_width + 20.0f, line_height * static_cast<float>(items.size()) + 10.0f);
}

void draw_graph(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size,
               const std::vector<float>& data, ImU32 color, ImU32 bg_color) {
  if (data.empty()) return;

  draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bg_color);

  float min_val = *std::min_element(data.begin(), data.end());
  float max_val = *std::max_element(data.begin(), data.end());
  float range = max_val - min_val;
  if (range == 0.0f) range = 1.0f;

  float step_x = size.x / static_cast<float>(data.size() - 1);

  for (size_t i = 0; i < data.size() - 1; ++i) {
    float x1 = pos.x + step_x * static_cast<float>(i);
    float y1 = pos.y + size.y - (data[i] - min_val) / range * size.y;
    float x2 = pos.x + step_x * static_cast<float>(i + 1);
    float y2 = pos.y + size.y - (data[i + 1] - min_val) / range * size.y;

    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, 2.0f);
  }
}

bool render_dropdown(const std::string& label, const std::vector<std::string>& items,
                    int& current_index, float width) {
  if (!ImGui::BeginCombo(label.c_str(), items[current_index].c_str())) return false;

  for (size_t i = 0; i < items.size(); ++i) {
    bool is_selected = (static_cast<size_t>(current_index) == i);
    if (ImGui::Selectable(items[i].c_str(), is_selected)) {
      current_index = static_cast<int>(i);
      ImGui::EndCombo();
      return true;
    }
    if (is_selected) {
      ImGui::SetItemDefaultFocus();
    }
  }

  ImGui::EndCombo();
  return false;
}

bool render_color_picker(const std::string& label, ImVec4& color) {
  float col[4] = { color.x, color.y, color.z, color.w };
  if (ImGui::ColorEdit4(label.c_str(), col)) {
    color = ImVec4(col[0], col[1], col[2], col[3]);
    return true;
  }
  return false;
}

void render_help_tooltip(const std::string& text, float max_width) {
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(max_width);
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

void render_progress_bar(const std::string& label, float fraction, const ImVec2& size) {
  ImGui::ProgressBar(fraction, size, label.c_str());
}

void render_timed_tooltip(const std::string& text, float duration_seconds, ImU32 color) {
  static std::string tooltip_text;
  static float timer = 0.0f;

  ImGuiIO& io = ImGui::GetIO();

  if (!text.empty()) {
    tooltip_text = text;
    timer = duration_seconds;
  }

  if (timer > 0.0f && !tooltip_text.empty()) {
    timer -= io.DeltaTime;

    ImGui::SetNextWindowPos(ImVec2(io.MousePos.x + 16.0f, io.MousePos.y + 16.0f));
    ImGui::Begin("##timed_tooltip", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                 ImGuiWindowFlags_NoInputs);
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(color), "%s", tooltip_text.c_str());
    ImGui::End();
  }
}

void render_overlay_button(const std::string& label, const ImVec2& pos, const ImVec2& size,
                          ImU32 color, ImU32 hover_color, std::function<void()> on_click) {
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();

  ImVec2 max_pos(pos.x + size.x, pos.y + size.y);
  bool hovered = ImGui::IsMouseHoveringRect(pos, max_pos);
  bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

  draw_list->AddRectFilled(pos, max_pos, hovered ? hover_color : color, 4.0f);

  ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
  draw_list->AddText(
    ImVec2(pos.x + (size.x - text_size.x) * 0.5f,
           pos.y + (size.y - text_size.y) * 0.5f),
    IM_COL32(255, 255, 255, 255),
    label.c_str()
  );

  if (clicked && on_click) {
    on_click();
  }
}

bool render_context_menu(const std::string& name,
                         const std::vector<std::pair<std::string, std::function<void()>>>& items) {
  if (!ImGui::BeginPopupContextItem(name.c_str())) return false;

  for (const auto& [label, action] : items) {
    if (ImGui::MenuItem(label.c_str())) {
      if (action) action();
      ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
      return true;
    }
  }

  ImGui::EndPopup();
  return false;
}

void render_border_frame(const ImVec2& pos, const ImVec2& size, ImU32 color, float thickness) {
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();
  draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), color, 0.0f, 0, thickness);
}

bool is_mouse_over_rect(const ImVec2& rect_min, const ImVec2& rect_max) {
  return ImGui::IsMouseHoveringRect(rect_min, rect_max);
}

void clamp_to_monitor(int& x, int& y, int w, int h, int monitor_w, int monitor_h) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x + w > monitor_w) x = monitor_w - w;
  if (y + h > monitor_h) y = monitor_h - h;
}

static float g_default_font_size = 14.0f;

float get_default_font_size() {
  return g_default_font_size;
}

void set_default_font_size(float size) {
  g_default_font_size = size;
}

ImVec2 get_font_dimensions(float size, const char* text) {
  return ImGui::CalcTextSize(text);
}

} // namespace exile::ui