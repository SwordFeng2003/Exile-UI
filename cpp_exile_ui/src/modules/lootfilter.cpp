#include "modules/lootfilter.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/clipboard_handler.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <fstream>
#include <algorithm>

namespace exile::modules {

void Lootfilter::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  modifier_key_ = cfg.read<std::string>("FilterSpoon", "modifier-key", "Alt");
  color_accent_ = cfg.read<std::string>("FilterSpoon", "accent-color", "00FF00");

  int profile_count = cfg.read<int>("FilterSpoon", "profile-count", 1);
  for (int i = 0; i < profile_count; ++i) {
    load_profile(i);
  }
}

void Lootfilter::shutdown() {
  if (editor_visible_) hide_editor();
}

void Lootfilter::render_editor() {
  if (!editor_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("filter editor").c_str(), &editor_visible_,
               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu(lang.translate("file").c_str())) {
      if (ImGui::MenuItem(lang.translate("apply to game").c_str())) {
        apply_to_game();
      }
      if (ImGui::MenuItem(lang.translate("restore").c_str())) {
        restore_from_game();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(lang.translate("edit").c_str())) {
      if (ImGui::MenuItem(lang.translate("add rule").c_str())) {
        FilterRule rule;
        rule.name = "New Rule";
        add_rule(rule);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  if (profiles_.empty()) {
    ImGui::End();
    return;
  }

  auto& profile = profiles_[active_profile_];

  ImGui::Text("%s: %s", lang.translate("active profile").c_str(), profile.name.c_str());

  if (ImGui::BeginTable("##rules_table", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY)) {
    ImGui::TableSetupColumn(lang.translate("rule name").c_str());
    ImGui::TableSetupColumn(lang.translate("action").c_str(), ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn(lang.translate("item class").c_str(), ImGuiTableColumnFlags_WidthFixed, 120);
    ImGui::TableSetupColumn(lang.translate("min level").c_str(), ImGuiTableColumnFlags_WidthFixed, 60);
    ImGui::TableSetupColumn(lang.translate("enabled").c_str(), ImGuiTableColumnFlags_WidthFixed, 50);
    ImGui::TableHeadersRow();

    for (size_t i = 0; i < profile.rules.size(); ++i) {
      auto& rule = profile.rules[i];
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%s", rule.name.c_str());

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%s", rule.action.c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%s", rule.item_class.c_str());

      ImGui::TableSetColumnIndex(3);
      ImGui::Text("%d", rule.min_level);

      ImGui::TableSetColumnIndex(4);
      bool enabled = rule.enabled;
      ImGui::PushID(static_cast<int>(i));
      if (ImGui::Checkbox("##enabled", &enabled)) {
        rule.enabled = enabled;
      }
      ImGui::PopID();
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void Lootfilter::show_editor() {
  editor_visible_ = true;
}

void Lootfilter::hide_editor() {
  editor_visible_ = false;
}

bool Lootfilter::load_profile(int index) {
  auto& cfg = core::Application::instance().config_manager();
  auto prefix = "profile" + std::to_string(index + 1);

  FilterProfile profile;
  profile.name = cfg.read<std::string>("FilterSpoon", prefix + "-name", "Profile " + std::to_string(index + 1));
  profile.game_filter_name = cfg.read<std::string>("FilterSpoon", prefix + "-filter", "");

  int rule_count = cfg.read<int>("FilterSpoon", prefix + "-rule-count", 0);
  for (int i = 0; i < rule_count; ++i) {
    FilterRule rule;
    auto rp = prefix + "-r" + std::to_string(i + 1);
    rule.name = cfg.read<std::string>("FilterSpoon", rp + "-name", "");
    rule.action = cfg.read<std::string>("FilterSpoon", rp + "-action", "Show");
    rule.item_class = cfg.read<std::string>("FilterSpoon", rp + "-class", "");
    rule.enabled = cfg.read<bool>("FilterSpoon", rp + "-enabled", true);
    profile.rules.push_back(rule);
  }

  if (index >= static_cast<int>(profiles_.size())) {
    profiles_.push_back(std::move(profile));
  } else {
    profiles_[index] = std::move(profile);
  }

  return true;
}

bool Lootfilter::save_profile(int index) {
  if (index >= static_cast<int>(profiles_.size())) return false;

  auto& cfg = core::Application::instance().config_manager();
  auto prefix = "profile" + std::to_string(index + 1);
  auto& profile = profiles_[index];

  cfg.write("FilterSpoon", prefix + "-name", profile.name);
  cfg.write("FilterSpoon", "profile-count", static_cast<int>(profiles_.size()));

  return true;
}

void Lootfilter::apply_to_game() {
  update_pending_ = true;
  auto filter_text = generate_filter_text();

  std::string folder_path = "data/filters/";
  auto& cfg = core::Application::instance().config_manager();
  auto name = cfg.read<std::string>("FilterSpoon", "filter-name",
    profiles_[active_profile_].game_filter_name);

  if (name.empty()) name = profiles_[active_profile_].name;

  std::ofstream file(folder_path + name + ".filter");
  if (file.is_open()) {
    file << filter_text;
    file.close();
    update_applied_ = true;
    update_pending_ = false;
  }
}

void Lootfilter::restore_from_game() {
  update_pending_ = false;
  update_applied_ = false;
}

void Lootfilter::add_rule(const FilterRule& rule) {
  if (active_profile_ < static_cast<int>(profiles_.size())) {
    profiles_[active_profile_].rules.push_back(rule);
  }
}

void Lootfilter::remove_rule(size_t index) {
  if (active_profile_ < static_cast<int>(profiles_.size()) &&
      index < profiles_[active_profile_].rules.size()) {
    profiles_[active_profile_].rules.erase(
      profiles_[active_profile_].rules.begin() + index);
  }
}

void Lootfilter::update_rule(size_t index, const FilterRule& rule) {
  if (active_profile_ < static_cast<int>(profiles_.size()) &&
      index < profiles_[active_profile_].rules.size()) {
    profiles_[active_profile_].rules[index] = rule;
  }
}

void Lootfilter::move_rule_up(size_t index) {
  if (index > 0 && active_profile_ < static_cast<int>(profiles_.size())) {
    auto& rules = profiles_[active_profile_].rules;
    std::swap(rules[index], rules[index - 1]);
  }
}

void Lootfilter::move_rule_down(size_t index) {
  if (active_profile_ < static_cast<int>(profiles_.size())) {
    auto& rules = profiles_[active_profile_].rules;
    if (index + 1 < rules.size()) {
      std::swap(rules[index], rules[index + 1]);
    }
  }
}

void Lootfilter::parse_clipboard_item() {
}

void Lootfilter::create_rule_from_item() {
}

std::string Lootfilter::generate_filter_text() const {
  if (active_profile_ >= static_cast<int>(profiles_.size())) return "";

  std::string result;
  result += "# Generated by Exile UI - FilterSpoon\n";
  result += "# Profile: " + profiles_[active_profile_].name + "\n\n";

  result += "Show\n";
  result += "  SetBorderColor 0 0 0 0\n";
  result += "  SetBackgroundColor 0 0 0 0\n\n";

  for (auto& rule : profiles_[active_profile_].rules) {
    if (!rule.enabled) continue;
    result += rule_to_filter_line(rule);
  }

  return result;
}

std::string Lootfilter::rule_to_filter_line(const FilterRule& rule) const {
  std::string result;
  result += rule.action + "\n";
  if (!rule.item_class.empty()) {
    result += "  Class \"" + rule.item_class + "\"\n";
  }
  if (rule.min_level > 0) {
    result += "  AreaLevel >= " + std::to_string(rule.min_level) + "\n";
  }
  result += "  SetFontSize " + std::to_string(rule.font_size) + "\n";
  result += "\n";
  return result;
}

} // namespace exile::modules