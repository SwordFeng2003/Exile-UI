#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct MapRun {
  std::string map_name;
  std::string map_id;
  std::string date_time;
  int tier = 0;
  int kills = 0;
  int deaths = 0;
  float duration_seconds = 0.0f;
  std::vector<std::string> mods;
  std::vector<std::string> mechanics;
  std::string notes;
  bool boss_killed = false;
  int portal_count = 6;
};

struct MapRunSession {
  std::string date;
  std::vector<MapRun> runs;
  int total_kills = 0;
  int total_deaths = 0;
  int total_maps = 0;
};

class MapTracker {
public:
  void initialize();
  void shutdown();

  void render_panel();
  void render_logs_window();
  void show_panel();
  void hide_panel();
  bool is_panel_visible() const { return panel_visible_; }

  void start_map(const std::string& map_id, const std::string& map_name, int tier);
  void end_map();
  void update_kills(int count);
  void increment_deaths();
  void add_note(const std::string& note);

  void add_mod(const std::string& mod);
  void remove_mod(const std::string& mod);
  void add_mechanic(const std::string& mechanic);

  void save_run();
  void load_history();
  void export_data(const std::string& format = "json");

  const MapRun& current_run() const { return current_run_; }
  bool is_tracking() const { return tracking_; }
  void pause_tracking(bool pause) { paused_ = pause; }
  bool is_paused() const { return paused_; }

  void on_tab_press();
  void on_hideout_check();
  bool is_in_hideout() const { return in_hideout_; }

  void set_loot_tracking(bool enabled) { loot_tracking_ = enabled; }

private:
  MapRun current_run_;
  std::vector<MapRunSession> history_;
  bool panel_visible_ = false;
  bool tracking_ = false;
  bool paused_ = false;
  bool in_hideout_ = false;
  bool loot_tracking_ = false;
  uint64_t map_start_time_ = 0;
  int kill_count_start_ = 0;
  std::string map_start_area_;
};

} // namespace exile::modules