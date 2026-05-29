#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct FilterRule {
  std::string name;
  std::string action;
  std::string item_class;
  std::string base_type;
  int min_level = 0;
  int max_level = 100;
  int min_quality = 0;
  int max_quality = 20;
  std::string rarity;
  int sockets_min = 0;
  int sockets_max = 6;
  std::string links;
  bool identified = false;
  bool corrupted = false;
  bool influenced = false;
  std::string enchantment;
  std::string mod_filter;
  std::string sound;
  int minimap_icon = 0;
  int minimap_size = 1;
  ImVec4 text_color = ImVec4(1,1,1,1);
  ImVec4 border_color = ImVec4(0,0,0,1);
  ImVec4 background_color = ImVec4(0,0,0,0);
  int font_size = 32;
  bool enabled = true;
};

struct FilterProfile {
  std::string name;
  std::string game_filter_name;
  std::vector<FilterRule> rules;
  bool is_default = false;
};

class Lootfilter {
public:
  void initialize();
  void shutdown();

  void render_editor();
  void show_editor();
  void hide_editor();
  bool is_visible() const { return editor_visible_; }

  bool load_profile(int index);
  bool save_profile(int index);
  void apply_to_game();
  void restore_from_game();

  void add_rule(const FilterRule& rule);
  void remove_rule(size_t index);
  void update_rule(size_t index, const FilterRule& rule);
  void move_rule_up(size_t index);
  void move_rule_down(size_t index);

  void parse_clipboard_item();
  void create_rule_from_item();

  const std::vector<FilterProfile>& profiles() const { return profiles_; }
  int active_profile() const { return active_profile_; }
  void set_active_profile(int index) { active_profile_ = index; }

  void set_modifier_key(const std::string& key) { modifier_key_ = key; }
  void set_color_accent(const std::string& hex) { color_accent_ = hex; }

private:
  std::string generate_filter_text() const;
  std::string rule_to_filter_line(const FilterRule& rule) const;

  std::vector<FilterProfile> profiles_;
  int active_profile_ = 0;
  bool editor_visible_ = false;
  bool update_pending_ = false;
  bool update_applied_ = false;
  std::string modifier_key_ = "Alt";
  std::string color_accent_ = "00FF00";

  nlohmann::json modifications_;
  std::vector<std::string> search_history_;
  int search_history_index_ = 0;
};

} // namespace exile::modules