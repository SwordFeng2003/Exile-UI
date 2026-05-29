#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct AtlasNode {
  std::string id;
  std::string name;
  std::string description;
  int x = 0;
  int y = 0;
  int radius = 0;
  int group = 0;
  int orbit = 0;
  int orbit_index = 0;
  bool allocated = false;
  bool highlight = false;
};

struct AtlasTreeNode {
  std::string name;
  std::string stats;
  std::string reminder_text;
  std::string flavor_text;
  std::vector<AtlasTreeNode> children;
  bool is_notable = false;
  bool is_keystone = false;
  bool is_mastery = false;
  bool allocated = false;
};

class Statlas {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void load_atlas_data();
  void calculate_orbit_positions();

  void allocate_node(const std::string& node_id);
  void deallocate_node(const std::string& node_id);
  void refund_all();
  int total_allocated() const;
  int max_passive_points() const { return max_points_; }

  void build_tree_view();
  float sum_circle(int start, int count) const;

  void set_ui_scaling(float scale) { ui_scaling_ = scale; }
  void set_curr_connections(bool v) { curved_connections_ = v; }
  void set_max_travel_nodes(int nodes) { max_travel_nodes_ = nodes; }

  void on_tree_search(const std::string& query);

private:
  std::unordered_map<std::string, AtlasNode> nodes_;
  std::vector<AtlasTreeNode> tree_roots_;
  bool overlay_visible_ = false;
  int max_points_ = 132;
  int max_travel_nodes_ = 6;
  float ui_scaling_ = 1.0f;
  bool curved_connections_ = true;

  nlohmann::json atlas_data_;
  nlohmann::json search_results_;
};

} // namespace exile::modules