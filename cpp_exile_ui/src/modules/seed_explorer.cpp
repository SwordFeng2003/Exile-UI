#include "modules/seed_explorer.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace exile::modules {

void SeedExplorer::initialize() {
  load_rule_groups();
  load_seed_data();

  auto& cfg = core::Application::instance().config_manager();
  border_thickness_ = cfg.read<float>("Seed-explorer", "border-thickness", 1.0f);
}

void SeedExplorer::shutdown() {
}

void SeedExplorer::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("seed explorer").c_str(), &overlay_visible_);

  if (ImGui::Button(lang.translate("refresh").c_str())) refresh_tree();
  ImGui::SameLine();
  if (ImGui::Button(lang.translate("collapse all").c_str())) collapse_all();
  ImGui::SameLine();
  if (ImGui::Button(lang.translate("expand all").c_str())) expand_all();

  ImGui::Separator();

  for (size_t i = 0; i < rule_groups_.size(); ++i) {
    auto& group = rule_groups_[i];
    ImGui::PushID(static_cast<int>(i));

    bool toggled = group.result;
    if (ImGui::Checkbox(group.name.c_str(), &toggled)) {
      toggle_group(i);
    }

    if (group.result) {
      ImGui::Indent();
      for (auto& child : group.children) {
        ImGui::BulletText("%s: %s", child.name.c_str(), child.result ? "PASS" : "FAIL");
      }
      ImGui::Unindent();
    }

    ImGui::PopID();
  }

  ImGui::End();
}

void SeedExplorer::show_overlay() { overlay_visible_ = true; }
void SeedExplorer::hide_overlay() { overlay_visible_ = false; }

void SeedExplorer::refresh_tree() {
  populate_seeds();
}

void SeedExplorer::populate_seeds() {
}

void SeedExplorer::reset_seeds() {
  for (auto& group : rule_groups_) {
    group.result = false;
    for (auto& child : group.children) {
      child.result = false;
    }
  }
}

void SeedExplorer::load_rule_groups() {
  auto& res = core::Application::instance().resource_manager();

  if (res.has_json("seed_rules")) {
    try {
      auto data = nlohmann::json::parse(res.get_json_string("seed_rules"));
      for (auto& item : data) {
        SeedRuleGroup group;
        group.name = item.value("name", "");
        group.source = item.value("source", "");

        if (item.contains("children")) {
          for (auto& child : item["children"]) {
            SeedRule rule;
            rule.name = child.value("name", "");
            rule.condition_type = child.value("type", "");
            group.children.push_back(rule);
          }
        }

        rule_groups_.push_back(group);
      }
    } catch (const std::exception&) {}
  }
}

void SeedExplorer::load_seed_data() {
  auto& res = core::Application::instance().resource_manager();
  if (res.has_json("seeds")) {
    seed_data_.clear();
    try {
      auto data = nlohmann::json::parse(res.get_json_string("seeds"));
      for (auto& s : data) {
        seed_data_.push_back(s.get<std::string>());
      }
    } catch (const std::exception&) {}
  }
}

void SeedExplorer::toggle_group(size_t index) {
  if (index < rule_groups_.size()) {
    rule_groups_[index].result = !rule_groups_[index].result;
  }
}

void SeedExplorer::collapse_all() {
  for (auto& group : rule_groups_) {
    group.result = false;
  }
}

void SeedExplorer::expand_all() {
  for (auto& group : rule_groups_) {
    group.result = true;
  }
}

} // namespace exile::modules