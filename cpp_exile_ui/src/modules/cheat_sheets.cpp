#include "modules/cheat_sheets.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "core/resource_manager.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace exile::modules {

void CheatSheets::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  pos_x_ = cfg.read<int>("Cheat-sheets", "pos-x", 100);
  pos_y_ = cfg.read<int>("Cheat-sheets", "pos-y", 100);
  scale_ = cfg.read<float>("Cheat-sheets", "scale", 1.0f);
  opacity_ = cfg.read<float>("Cheat-sheets", "opacity", 1.0f);

  load_category("crafting");
  load_category("vendor_recipes");
}

void CheatSheets::shutdown() {
}

void CheatSheets::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::SetNextWindowPos(ImVec2(static_cast<float>(pos_x_), static_cast<float>(pos_y_)),
                          ImGuiCond_FirstUseEver);

  ImGui::Begin(lang.translate("cheat sheets").c_str(), &overlay_visible_,
               ImGuiWindowFlags_NoSavedSettings);

  for (size_t i = 0; i < categories_.size(); ++i) {
    if (ImGui::Button(categories_[i].name.c_str())) {
      navigate_to_category(static_cast<int>(i));
    }
    ImGui::SameLine();
  }
  ImGui::NewLine();

  ImGui::Separator();

  if (current_category_ < static_cast<int>(categories_.size())) {
    auto& cat = categories_[current_category_];

    ImGui::Text("%s (%d/%zu)", cat.name.c_str(),
               cat.current_index + 1, cat.entries.size());

    if (ImGui::Button("<")) cycle_entry(-1);
    ImGui::SameLine();
    if (ImGui::Button(">")) cycle_entry(1);

    if (cat.current_index < static_cast<int>(cat.entries.size())) {
      auto& entry = cat.entries[cat.current_index];

      ImGui::Separator();
      ImGui::TextWrapped("%s", entry.content.c_str());

      for (auto& line : entry.lines) {
        ImGui::BulletText("%s", line.c_str());
      }
    }
  }

  ImGui::End();
}

void CheatSheets::show_overlay() { overlay_visible_ = true; }
void CheatSheets::hide_overlay() { overlay_visible_ = false; }
void CheatSheets::close_overlay() { overlay_visible_ = false; }

void CheatSheets::load_category(const std::string& category) {
  auto& res = core::Application::instance().resource_manager();
  CheatSheetCategory cat;
  cat.name = category;
  cat.current_index = 0;

  if (res.has_json(category)) {
    auto data = nlohmann::json::parse(res.get_json_string(category));
    for (auto& item : data) {
      CheatSheetEntry entry;
      entry.label = item.value("label", "");
      entry.content = item.value("content", "");

      if (item.contains("lines")) {
        for (auto& l : item["lines"]) {
          entry.lines.push_back(l.get<std::string>());
        }
      }

      cat.entries.push_back(entry);
    }
  }

  categories_.push_back(std::move(cat));
}

void CheatSheets::cycle_entry(int direction) {
  if (current_category_ >= static_cast<int>(categories_.size())) return;
  auto& cat = categories_[current_category_];

  cat.current_index += direction;
  if (cat.current_index < 0) cat.current_index = static_cast<int>(cat.entries.size()) - 1;
  if (cat.current_index >= static_cast<int>(cat.entries.size())) cat.current_index = 0;
}

void CheatSheets::navigate_to_category(int index) {
  if (index >= 0 && index < static_cast<int>(categories_.size())) {
    current_category_ = index;
  }
}

void CheatSheets::set_position(int x, int y) {
  pos_x_ = x;
  pos_y_ = y;
}

void CheatSheets::set_scale(float scale) { scale_ = scale; }
void CheatSheets::set_opacity(float opacity) { opacity_ = opacity; }

} // namespace exile::modules