#include "modules/leveling_tracker.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/log_parser.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <fstream>
#include <chrono>

namespace exile::modules {

void LevelingTracker::initialize() {
  load_areas();
  load_gems();
  load_areas_database();
  load_gem_setups();

  auto& cfg = core::Application::instance().config_manager();
  auto_track_ = cfg.read<bool>("Leveling-tracker", "auto-track", false);
}

void LevelingTracker::shutdown() {
  destroy_overlay();
  stop_timer();
}

bool LevelingTracker::load_guide(int profile_index) {
  auto& res = core::Application::instance().resource_manager();
  auto& lang = core::Application::instance().language_manager();

  guide_ = LevelingGuide{};
  guide_.current_step = 0;

  auto& cfg = core::Application::instance().config_manager();
  guide_.character_class = cfg.read<std::string>("Leveling-tracker",
    "prof" + std::to_string(profile_index) + "-class", "");
  guide_.name = lang.translate("build name", profile_index);

  return true;
}

bool LevelingTracker::import_pob_build(const std::string& pob_code) {
  auto& lang = core::Application::instance().language_manager();

  guide_.gem_list.clear();
  guide_.item_list.clear();

  return true;
}

void LevelingTracker::advance_step() {
  if (guide_.current_step < static_cast<int>(guide_.steps.size()) - 1) {
    guide_.steps[guide_.current_step].completed = true;
    guide_.current_step++;
  }
}

void LevelingTracker::retreat_step() {
  if (guide_.current_step > 0) {
    guide_.current_step--;
    guide_.steps[guide_.current_step].completed = false;
  }
}

void LevelingTracker::mark_completed() {
  if (guide_.current_step < static_cast<int>(guide_.steps.size())) {
    guide_.steps[guide_.current_step].completed = true;
  }
}

void LevelingTracker::render_panel() {
  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("leveling tracker").c_str(), nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);

  if (guide_.steps.empty()) {
    ImGui::TextWrapped("%s", lang.translate("no guide loaded").c_str());
    ImGui::End();
    return;
  }

  ImGui::Text("Step %d/%zu", guide_.current_step + 1, guide_.steps.size());

  auto& step = guide_.steps[guide_.current_step];
  ImGui::Separator();
  ImGui::TextWrapped("%s", step.description.c_str());

  if (!step.reward.empty()) {
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Reward: %s", step.reward.c_str());
  }

  ImGui::End();
}

void LevelingTracker::render_gem_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::SetNextWindowPos(ImVec2(10, 100), ImGuiCond_FirstUseEver);
  ImGui::Begin("##gem_overlay", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

  ImGui::Text("%s", lang.translate("gem setups").c_str());
  ImGui::Separator();

  for (auto& gem : gem_setups_) {
    ImGui::BulletText("%s (Lvl %d)%s",
      gem.gem_name.c_str(), gem.required_level,
      gem.obtained ? " [OK]" : "");
  }

  ImGui::End();
}

void LevelingTracker::render_tree_overlay() {
  if (!overlay_visible_) return;
}

void LevelingTracker::render_timer() {
  if (!timer_running_) return;

  int minutes = static_cast<int>(elapsed_time_seconds_ / 60);
  int seconds = static_cast<int>(elapsed_time_seconds_) % 60;

  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::Begin("##leveling_timer", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
               ImGuiWindowFlags_NoSavedSettings);

  ImGui::Text("Timer: %02d:%02d", minutes, seconds);
  ImGui::End();
}

void LevelingTracker::toggle_overlay() {
  overlay_visible_ = !overlay_visible_;
}

void LevelingTracker::destroy_overlay() {
  overlay_visible_ = false;
}

void LevelingTracker::start_timer() {
  timer_running_ = true;
  elapsed_time_seconds_ = 0.0f;
}

void LevelingTracker::pause_timer() {
  timer_running_ = false;
}

void LevelingTracker::stop_timer() {
  timer_running_ = false;
  elapsed_time_seconds_ = 0.0f;
}

float LevelingTracker::elapsed_time() const {
  return elapsed_time_seconds_;
}

void LevelingTracker::set_character(const std::string& name, const std::string& char_class) {
  guide_.character_name = name;
  guide_.character_class = char_class;
}

void LevelingTracker::load_gem_setups() {
}

void LevelingTracker::load_passive_tree() {
}

void LevelingTracker::load_areas_database() {
  auto& res = core::Application::instance().resource_manager();
  if (res.has_json("areas")) {
    areas_db_ = nlohmann::json::parse(res.get_json_string("areas"));
  }
}

bool LevelingTracker::load_areas() {
  return true;
}

bool LevelingTracker::load_gems() {
  return true;
}

void LevelingTracker::handle_gem_cutting(const std::string& gem_name) {
}

void LevelingTracker::handle_gem_notes(const std::string& gem_name) {
}

} // namespace exile::modules