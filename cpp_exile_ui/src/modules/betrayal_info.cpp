#include "modules/betrayal_info.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/screen_checker.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>

namespace exile::modules {

void BetrayalInfo::initialize() {
  load_member_data();
  auto& cfg = core::Application::instance().config_manager();
  custom_bg_ = cfg.read<bool>("Betrayal-info", "custom-bg", false);
}

void BetrayalInfo::shutdown() {
}

void BetrayalInfo::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("betrayal info").c_str(), &overlay_visible_);

  static const char* division_names[] = {
    "Transportation", "Fortification", "Research", "Intervention"
  };

  for (int div = 0; div < 4; ++div) {
    if (ImGui::CollapsingHeader(division_names[div])) {
      for (auto& member : members_) {
        if (member.division == div) {
          ImGui::BulletText("%s (Rank %d, Trust %d)",
            member.name.c_str(), member.rank, member.trust);
          ImGui::SameLine();
          if (!member.reward.empty()) {
            ImGui::TextColored(ImVec4(0, 1, 1, 1), "[%s]", member.reward.c_str());
          }
        }
      }
    }
  }

  ImGui::End();
}

void BetrayalInfo::show_overlay() { overlay_visible_ = true; }
void BetrayalInfo::hide_overlay() { overlay_visible_ = false; }

void BetrayalInfo::load_member_data() {
  auto& res = core::Application::instance().resource_manager();

  members_ = {
    {"Aisling", 0, 0, 1, "", false, 0},
    {"Cameria", 1, 0, 1, "", false, 1},
    {"Elreon", 2, 0, 1, "", false, 2},
    {"Gravicius", 3, 0, 1, "", false, 3},
    {"Guff", 0, 0, 2, "", false, 0},
    {"Haku", 1, 0, 2, "", false, 1},
    {"Hillock", 2, 0, 2, "", false, 2},
    {"It That Fled", 3, 0, 2, "", false, 3},
    {"Janus", 0, 0, 2, "", false, 0},
    {"Jorgin", 1, 0, 2, "", false, 1},
    {"Korell", 2, 0, 2, "", false, 2},
    {"Leo", 3, 0, 2, "", false, 3},
    {"Riker", 0, 0, 3, "", false, 0},
    {"Rin", 1, 0, 3, "", false, 1},
    {"Tora", 2, 0, 3, "", false, 2},
    {"Vagan", 3, 0, 3, "", false, 3},
    {"Vorici", 2, 0, 3, "", false, 2},
  };
}

void BetrayalInfo::parse_betrayal_ui() {
}

void BetrayalInfo::update_member(int index, const BetrayalMember& member) {
  if (index >= 0 && index < static_cast<int>(members_.size())) {
    members_[index] = member;
  }
}

void BetrayalInfo::execute_action(const std::string& action) {
}

void BetrayalInfo::highlight_rewards() {
}

BetrayalMember* BetrayalInfo::find_member(const std::string& name) {
  for (auto& m : members_) {
    if (m.name == name) return &m;
  }
  return nullptr;
}

} // namespace exile::modules