#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct StashTab {
  std::string name;
  int index = 0;
  std::string type;
  bool is_public = false;
  std::string price_tag;
};

struct StashItem {
  std::string name;
  std::string base_type;
  float chaos_value = 0.0f;
  float divine_value = 0.0f;
  float trend = 0.0f;
  bool highlighted = false;
};

class StashNinja {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  void close_overlay();
  bool is_visible() const { return overlay_visible_; }

  void update_prices(const std::string& league);
  bool fetch_prices(const std::string& category = "currency");
  bool load_cached_prices(const std::string& category);
  void save_prices_to_cache(const std::string& category);

  const StashItem* find_item(const std::string& name) const;
  float get_price(const std::string& item_name) const;

  void set_active_tab(int index);
  void set_stash_width(int width) { stash_width_ = width; }

  void refresh_overlay();
  void handle_hover(int mouse_x, int mouse_y);

  bool needs_price_update() const;
  void mark_price_updated();

private:
  std::unordered_map<std::string, StashItem> items_;
  std::unordered_map<std::string, std::string> item_names_;
  std::vector<StashTab> tabs_;
  int active_tab_ = 0;
  int stash_width_ = 600;
  bool overlay_visible_ = false;
  std::string active_league_;
  uint64_t last_update_time_ = 0;
  nlohmann::json price_cache_;
};

} // namespace exile::modules