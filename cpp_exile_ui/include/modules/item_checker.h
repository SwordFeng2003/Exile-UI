#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace exile::modules {

struct ItemStat {
  std::string name;
  std::string value;
  std::string range;
  bool is_crafted = false;
  bool is_fractured = false;
  bool is_implicit = false;
  bool is_highlighted = false;
  int tier = 0;
};

struct ItemInfo {
  std::string name;
  std::string base_type;
  std::string rarity;
  std::string item_class;
  int item_level = 0;
  int required_level = 0;
  int required_str = 0;
  int required_dex = 0;
  int required_int = 0;
  int sockets = 0;
  int stack_size = 0;
  bool identified = true;
  bool corrupted = false;
  std::string influence;
  std::vector<ItemStat> implicits;
  std::vector<ItemStat> explicits;
  std::vector<ItemStat> crafted_mods;
  std::vector<ItemStat> fractured_mods;
  std::string cluster_enchant;
  int map_tier = 0;
};

class ItemChecker {
public:
  void initialize();
  void shutdown();

  bool parse_clipboard_item();
  void render_tooltip();
  void close_tooltip();
  bool is_tooltip_visible() const { return tooltip_visible_; }

  void highlight_stat(const std::string& stat_name);
  void clear_highlights();

  const ItemInfo& current_item() const { return current_item_; }
  void set_activation_mode(const std::string& mode) { activation_mode_ = mode; }
  void set_compare_enabled(bool enabled) { compare_enabled_ = enabled; }

  void handle_gear_parse();
  void update_gear_slot(const std::string& slot);

private:
  void parse_item_name(const std::string& line);
  void parse_item_stats(const std::string& line);
  void parse_requirements(const std::string& line);
  void determine_tiers();

  ItemInfo current_item_;
  bool tooltip_visible_ = false;
  std::string activation_mode_ = "toggle";
  bool compare_enabled_ = false;
  int tooltip_x_ = 0;
  int tooltip_y_ = 0;
  int tooltip_w_ = 400;
  int tooltip_h_ = 0;

  std::unordered_map<std::string, int> highlighted_stats_;
};

} // namespace exile::modules