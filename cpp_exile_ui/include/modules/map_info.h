#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct MapModInfo {
  std::string name;
  std::string description;
  std::string icon_path;
  float difficulty = 0.0f;
  float reward_value = 0.0f;
  std::vector<std::string> tags;
};

struct MapBaseInfo {
  std::string name;
  int tier = 0;
  std::string boss_name;
  std::string boss_guide;
  std::string layout_type;
  std::vector<std::string> favored_by;
  std::vector<std::string> divination_cards;
};

struct CraftingRecipe {
  std::string name;
  std::string effect;
  std::string cost;
  std::string location;
  std::vector<std::string> unveiling_sources;
};

struct HarvestCraft {
  std::string name;
  std::string category;
  std::string description;
  std::string cost;
  std::string color;
};

class MapInfo {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void search_map(const std::string& query);
  void search_mod(const std::string& query);
  void search_recipe(const std::string& query);

  const MapModInfo* get_mod(const std::string& name) const;
  const MapBaseInfo* get_map(const std::string& name) const;

  void load_database();
  void load_recipes();
  void load_harvest_crafts();

  void check_map_mods();

private:
  std::unordered_map<std::string, MapModInfo> mods_;
  std::unordered_map<std::string, MapBaseInfo> maps_;
  std::vector<CraftingRecipe> recipes_;
  std::vector<HarvestCraft> harvest_crafts_;
  bool overlay_visible_ = false;
  std::string search_query_;
  int window_x_ = 0;
  int window_y_ = 0;

  nlohmann::json recipe_cache_;
};

} // namespace exile::modules