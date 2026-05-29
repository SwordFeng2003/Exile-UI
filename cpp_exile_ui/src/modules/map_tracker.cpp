#include "modules/map_tracker.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/log_parser.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <chrono>
#include <ctime>
#include <fstream>

namespace exile::modules {

void MapTracker::initialize() {
  load_history();

  auto& cfg = core::Application::instance().config_manager();
  loot_tracking_ = cfg.read<bool>("Map-tracker", "loot-tracking", false);
}

void MapTracker::shutdown() {
  if (tracking_) {
    end_map();
  }
}

void MapTracker::render_panel() {
  if (!panel_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("map tracker").c_str(), &panel_visible_,
               ImGuiWindowFlags_NoSavedSettings);

  if (tracking_) {
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", lang.translate("tracking").c_str());
  } else {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "%s", lang.translate("idle").c_str());
  }

  if (!tracking_) {
    if (ImGui::Button(lang.translate("start tracking").c_str())) {
      ImGui::End();
      return;
    }
  } else {
    if (ImGui::Button(lang.translate("stop tracking").c_str())) {
      end_map();
    }

    ImGui::SameLine();
    bool p = paused_;
    if (ImGui::Checkbox(lang.translate("pause").c_str(), &p)) {
      pause_tracking(p);
    }
  }

  ImGui::Separator();

  ImGui::Text("%s: %s", lang.translate("current map").c_str(), current_run_.map_name.c_str());
  ImGui::Text("%s: %d", lang.translate("tier").c_str(), current_run_.tier);
  ImGui::Text("%s: %d", lang.translate("kills").c_str(), current_run_.kills);
  ImGui::Text("%s: %d", lang.translate("deaths").c_str(), current_run_.deaths);

  if (tracking_) {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch()).count() - map_start_time_;
    int minutes = static_cast<int>(elapsed / 60);
    int seconds = static_cast<int>(elapsed % 60);
    ImGui::Text("%s: %02d:%02d", lang.translate("duration").c_str(), minutes, seconds);
  }

  if (!current_run_.mods.empty()) {
    ImGui::SeparatorText(lang.translate("map mods").c_str());
    for (auto& mod : current_run_.mods) {
      ImGui::BulletText("%s", mod.c_str());
    }
  }

  ImGui::End();
}

void MapTracker::render_logs_window() {
  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("map history").c_str(), nullptr);

  if (ImGui::BeginTable("##history_table", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("Date");
    ImGui::TableSetupColumn("Map");
    ImGui::TableSetupColumn("Tier");
    ImGui::TableSetupColumn("Kills");
    ImGui::TableSetupColumn("Deaths");
    ImGui::TableSetupColumn("Duration");
    ImGui::TableSetupColumn("Boss");
    ImGui::TableHeadersRow();

    for (auto& session : history_) {
      for (auto& run : session.runs) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", run.date_time.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", run.map_name.c_str());
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d", run.tier);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%d", run.kills);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%d", run.deaths);
        ImGui::TableSetColumnIndex(5);
        int min = static_cast<int>(run.duration_seconds / 60);
        int sec = static_cast<int>(run.duration_seconds) % 60;
        ImGui::Text("%02d:%02d", min, sec);
        ImGui::TableSetColumnIndex(6);
        ImGui::Text("%s", run.boss_killed ? "Yes" : "No");
      }
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void MapTracker::show_panel() { panel_visible_ = true; }
void MapTracker::hide_panel() { panel_visible_ = false; }

void MapTracker::start_map(const std::string& map_id, const std::string& map_name, int tier) {
  current_run_ = MapRun{};
  current_run_.map_id = map_id;
  current_run_.map_name = map_name;
  current_run_.tier = tier;

  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M");
  current_run_.date_time = oss.str();

  map_start_time_ = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::high_resolution_clock::now().time_since_epoch()).count();

  tracking_ = true;
  paused_ = false;
}

void MapTracker::end_map() {
  if (!tracking_) return;

  auto now = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  current_run_.duration_seconds = static_cast<float>(now - map_start_time_);

  save_run();
  tracking_ = false;
}

void MapTracker::update_kills(int count) {
  current_run_.kills = count;
}

void MapTracker::increment_deaths() {
  current_run_.deaths++;
}

void MapTracker::add_note(const std::string& note) {
  current_run_.notes += note + "\n";
}

void MapTracker::add_mod(const std::string& mod) {
  current_run_.mods.push_back(mod);
}

void MapTracker::remove_mod(const std::string& mod) {
  auto it = std::find(current_run_.mods.begin(), current_run_.mods.end(), mod);
  if (it != current_run_.mods.end()) {
    current_run_.mods.erase(it);
  }
}

void MapTracker::add_mechanic(const std::string& mechanic) {
  current_run_.mechanics.push_back(mechanic);
}

void MapTracker::save_run() {
  MapRunSession session;
  session.date = current_run_.date_time;
  session.runs.push_back(current_run_);
  session.total_maps = 1;
  session.total_kills = current_run_.kills;
  session.total_deaths = current_run_.deaths;

  history_.push_back(session);
}

void MapTracker::load_history() {
  auto& cfg = core::Application::instance().config_manager();
}

void MapTracker::export_data(const std::string& format) {
  std::ofstream file("data/map_history." + format);
  if (!file.is_open()) return;

  if (format == "json") {
    file << "{ \"sessions\": [] }";
  } else if (format == "csv") {
    file << "Date,Map,Tier,Kills,Deaths,Duration,Boss\n";
    for (auto& session : history_) {
      for (auto& run : session.runs) {
        file << run.date_time << "," << run.map_name << "," << run.tier << ","
             << run.kills << "," << run.deaths << "," << run.duration_seconds << ","
             << (run.boss_killed ? "Yes" : "No") << "\n";
      }
    }
  }
}

void MapTracker::on_tab_press() {
}

void MapTracker::on_hideout_check() {
}

} // namespace exile::modules