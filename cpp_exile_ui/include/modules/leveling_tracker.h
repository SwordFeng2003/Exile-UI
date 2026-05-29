#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct LevelingStep {
  std::string area_id;
  std::string action;
  std::string description;
  std::string reward;
  bool completed = false;
};

struct LevelingGuide {
  std::string name;
  std::string character_class;
  std::string character_name;
  std::vector<LevelingStep> steps;
  int current_step = 0;
  std::vector<std::string> gem_list;
  std::vector<std::string> item_list;
};

struct GemSetup {
  std::string gem_name;
  std::string socket_group;
  std::string location;
  int required_level = 0;
  bool obtained = false;
};

class LevelingTracker {
public:
  void initialize();
  void shutdown();

  bool load_guide(int profile_index);
  bool import_pob_build(const std::string& pob_code);
  void advance_step();
  void retreat_step();
  void mark_completed();

  void render_panel();
  void render_gem_overlay();
  void render_tree_overlay();
  void render_timer();

  void toggle_overlay();
  void destroy_overlay();
  bool is_overlay_visible() const { return overlay_visible_; }

  void start_timer();
  void pause_timer();
  void stop_timer();
  float elapsed_time() const;

  void auto_track_enable(bool enable) { auto_track_ = enable; }
  bool auto_track() const { return auto_track_; }

  const LevelingGuide& current_guide() const { return guide_; }
  void set_character(const std::string& name, const std::string& char_class);

  void load_gem_setups();
  void load_passive_tree();
  void load_areas_database();

  void handle_gem_cutting(const std::string& gem_name);
  void handle_gem_notes(const std::string& gem_name);

private:
  bool load_areas();
  bool load_gems();

  LevelingGuide guide_;
  std::vector<GemSetup> gem_setups_;
  std::vector<std::string> passive_tree_data_;
  bool overlay_visible_ = false;
  bool auto_track_ = false;
  bool timer_running_ = false;
  float elapsed_time_seconds_ = 0.0f;

  nlohmann::json areas_db_;
  nlohmann::json gems_db_;
  std::unordered_map<std::string, std::string> area_name_map_;
};

} // namespace exile::modules