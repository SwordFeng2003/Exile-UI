#include "modules/sanctum_planner.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/screen_checker.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <fstream>

namespace exile::modules {

void SanctumPlanner::initialize() {
  load_relics();

  auto& cfg = core::Application::instance().config_manager();
  zoom_ = cfg.read<float>("Sanctum-planner", "zoom", 1.0f);
}

void SanctumPlanner::shutdown() {
  save_relics();
}

void SanctumPlanner::render_main_window() {
  if (!visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("sanctum planner").c_str(), &visible_,
               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);

  if (locked_) {
    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "%s", lang.translate("sanctum locked").c_str());
  }

  if (ImGui::Button(lang.translate("scan floor").c_str())) {
    scan_floor();
  }
  ImGui::SameLine();
  if (ImGui::Button(lang.translate("toggle lock").c_str())) {
    locked_ = !locked_;
  }

  ImGui::Separator();

  render_floor_view();

  ImGui::End();
}

void SanctumPlanner::render_floor_view() {
  auto& lang = core::Application::instance().language_manager();

  if (floors_.empty()) {
    ImGui::Text("%s", lang.translate("no floor data").c_str());
    return;
  }

  if (current_floor_ >= static_cast<int>(floors_.size())) {
    current_floor_ = 0;
  }

  auto& floor = floors_[current_floor_];

  ImGui::Text("Floor %d/%zu", current_floor_ + 1, floors_.size());

  for (int r = 0; r < floor.row_count; ++r) {
    for (int c = 0; c < floor.col_count; ++c) {
      ImGui::PushID(r * 10 + c);

      auto& room = floor.grid[r][c];
      if (room.id.empty()) {
        ImGui::Button("##empty", ImVec2(40 * zoom_, 40 * zoom_));
      } else {
        ImU32 bg = IM_COL32(50, 50, 50, 255);
        if (room.completed) bg = IM_COL32(0, 80, 0, 255);
        else if (room.marked) bg = IM_COL32(80, 80, 0, 255);
        else if (room.revealed) bg = IM_COL32(60, 60, 80, 255);

        ImGui::PushStyleColor(ImGuiCol_Button, bg);
        if (ImGui::Button(room.id.c_str(), ImVec2(40 * zoom_, 40 * zoom_))) {
          mark_room(r, c, 1);
        }
        ImGui::PopStyleColor();
      }

      ImGui::PopID();
      ImGui::SameLine();
    }
    ImGui::NewLine();
  }
}

void SanctumPlanner::render_relics_window() {
  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("sanctum relics").c_str(), nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);

  for (auto& relic : relics_) {
    ImGui::BulletText("%s x%d", relic.name.c_str(), relic.quantity);
    if (!relic.modifier.empty()) {
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 1, 1), "  %s", relic.modifier.c_str());
    }
  }

  ImGui::End();
}

void SanctumPlanner::scan_floor() {
  scanning_ = true;
}

void SanctumPlanner::parse_floor_image() {
}

void SanctumPlanner::plan_optimal_path() {
}

void SanctumPlanner::mark_room(int row, int col, int mode) {
  if (current_floor_ >= static_cast<int>(floors_.size())) return;
  auto& floor = floors_[current_floor_];

  if (row >= 0 && row < floor.row_count && col >= 0 && col < floor.col_count) {
    auto& room = floor.grid[row][col];
    if (mode == 1) {
      room.marked = !room.marked;
    } else if (mode == 2) {
      room.completed = true;
    }
  }
}

void SanctumPlanner::toggle_room_reveal(int row, int col) {
  if (current_floor_ >= static_cast<int>(floors_.size())) return;
  auto& floor = floors_[current_floor_];

  if (row >= 0 && row < floor.row_count && col >= 0 && col < floor.col_count) {
    floor.grid[row][col].revealed = !floor.grid[row][col].revealed;
  }
}

void SanctumPlanner::load_relics() {
  relics_.clear();
}

void SanctumPlanner::save_relics() {
  auto& cfg = core::Application::instance().config_manager();
  cfg.write("Sanctum-planner", "relic-count", static_cast<int>(relics_.size()));
  for (size_t i = 0; i < relics_.size(); ++i) {
    auto prefix = "relic" + std::to_string(i + 1);
    cfg.write("Sanctum-planner", prefix + "-name", relics_[i].name);
    cfg.write("Sanctum-planner", prefix + "-quantity", relics_[i].quantity);
  }
}

void SanctumPlanner::add_relic(const SanctumRelic& relic) {
  relics_.push_back(relic);
}

void SanctumPlanner::remove_relic(size_t index) {
  if (index < relics_.size()) {
    relics_.erase(relics_.begin() + index);
  }
}

void SanctumPlanner::toggle_lock() {
  locked_ = !locked_;
}

} // namespace exile::modules