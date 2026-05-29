#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace exile::modules {

struct SeedRule {
  std::string name;
  std::string condition_type;
  std::string condition_value;
  bool result = false;
};

struct SeedRuleGroup {
  std::string name;
  std::string source;
  std::vector<SeedRule> children;
  bool result = false;
};

class SeedExplorer {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void refresh_tree();
  void populate_seeds();
  void reset_seeds();

  void load_rule_groups();
  void load_seed_data();

  void toggle_group(size_t index);
  void collapse_all();
  void expand_all();

  void set_border_thickness(float thickness) { border_thickness_ = thickness; }

private:
  std::vector<SeedRuleGroup> rule_groups_;
  std::vector<std::string> seed_data_;
  bool overlay_visible_ = false;
  float border_thickness_ = 1.0f;
};

} // namespace exile::modules