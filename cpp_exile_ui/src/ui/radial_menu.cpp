#include "ui/radial_menu.h"
#include "ui/theme.h"
#include "core/hotkey_manager.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>

namespace exile::ui {

int RadialMenu::position_to_grid(int pos) const {
  int row = (pos - 1) / kGridSize;
  int col = (pos - 1) % kGridSize;
  return row * kGridSize + col;
}

ImVec2 RadialMenu::grid_to_coords(int grid_pos, int cell_size, int margin) const {
  int row = grid_pos / kGridSize;
  int col = grid_pos % kGridSize;
  int total_size = cell_size + margin;
  int offset = total_size * (kGridSize - 1) / 2;
  return ImVec2(
    static_cast<float>(origin_x_ + col * total_size - offset),
    static_cast<float>(origin_y_ + row * total_size - offset)
  );
}

void RadialMenu::show(int x, int y, const std::vector<RadialEntry>& entries, bool animated) {
  origin_x_ = x;
  origin_y_ = y;
  entries_ = entries;
  animated_ = animated;
  anim_progress_ = animated_ ? 0.0f : 1.0f;
  visible_ = true;
  hovered_entry_ = -1;
}

void RadialMenu::show_submenu(int x, int y, const std::vector<RadialEntry>& entries) {
  show(x, y, entries, false);
}

void RadialMenu::hide() {
  visible_ = false;
  if (cancel_callback_) cancel_callback_();
}

bool RadialMenu::is_visible() const {
  return visible_;
}

void RadialMenu::render() {
  if (!visible_) return;

  if (animated_ && anim_progress_ < 1.0f) {
    anim_progress_ += 0.15f;
    if (anim_progress_ > 1.0f) anim_progress_ = 1.0f;
  }

  ImDrawList* draw_list = ImGui::GetForegroundDrawList();
  auto& theme = get_theme_colors();

  int total_size = cell_size_ + margin_;
  int offset = total_size * (kGridSize - 1) / 2;

  ImGui::SetNextWindowPos(ImVec2(
    static_cast<float>(origin_x_ - offset),
    static_cast<float>(origin_y_ - offset)
  ), ImGuiCond_Always);

  ImVec2 window_size(
    static_cast<float>(total_size * kGridSize),
    static_cast<float>(total_size * kGridSize)
  );

  ImGui::Begin("##radial_menu", &visible_,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
               ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);

  ImVec2 content_min = ImGui::GetWindowContentRegionMin();
  ImVec2 content_max = ImGui::GetWindowContentRegionMax();

  float center_x = (content_min.x + content_max.x) * 0.5f;
  float center_y = (content_min.y + content_max.y) * 0.5f;
  ImVec2 center(center_x, center_y);

  for (size_t i = 0; i < entries_.size() && i < static_cast<size_t>(kGridSize * kGridSize); ++i) {
    auto& entry = entries_[i];
    int grid_pos = position_to_grid(entry.position > 0 ? entry.position : static_cast<int>(i + 1));
    ImVec2 cell_pos = grid_to_coords(grid_pos, cell_size_, margin_);

    float scale = animated_ ? anim_progress_ : 1.0f;
    cell_pos.x += (center.x - cell_pos.x) * (1.0f - scale);
    cell_pos.y += (center.y - cell_pos.y) * (1.0f - scale);

    ImVec2 cell_min(cell_pos.x, cell_pos.y);
    ImVec2 cell_max(cell_pos.x + cell_size_, cell_pos.y + cell_size_);

    bool hovered = ImGui::IsMouseHoveringRect(cell_min, cell_max);
    if (hovered) hovered_entry_ = static_cast<int>(i);

    ImU32 bg_color = hovered ? to_im32(theme.surface_hover) : to_im32(theme.surface);
    draw_list->AddRectFilled(cell_min, cell_max, bg_color, theme.rounding);
    draw_list->AddRect(cell_min, cell_max, to_im32(theme.border), theme.rounding);

    ImVec2 text_pos(
      cell_pos.x + cell_size_ * 0.5f,
      cell_pos.y + cell_size_ * 0.5f - ImGui::GetFontSize() * 0.5f
    );
    ImVec2 text_size = ImGui::CalcTextSize(entry.label.c_str());
    draw_list->AddText(
      ImVec2(text_pos.x - text_size.x * 0.5f, text_pos.y),
      to_im32(theme.text_primary),
      entry.label.c_str()
    );
  }

  bool clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
  bool right_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right);

  if ((clicked || right_clicked) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    if (hovered_entry_ >= 0 && hovered_entry_ < static_cast<int>(entries_.size())) {
      auto& entry = entries_[hovered_entry_];
      if (right_clicked && entry.long_press_action) {
        entry.long_press_action();
      } else if (entry.action) {
        entry.action();
      }
      if (selection_callback_) selection_callback_(entry.label);
    } else {
      hide();
    }
  }

  ImGui::End();
}

} // namespace exile::ui