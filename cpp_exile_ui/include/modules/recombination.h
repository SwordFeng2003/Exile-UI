#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct RecombRecipe {
  std::string name;
  std::string item_class;
  std::string result;
  std::vector<std::string> ingredients;
  std::string description;
  bool crafted = false;
};

class Recombination {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void search_recipes(const std::string& query);
  void filter_by_result(const std::string& result_type);

  void parse_clipboard();
  void on_item_hover();
  void control_recipes();
  void check_validity();

  const RecombRecipe* find_recipe(const std::string& name) const;

  void load_database();

private:
  std::vector<RecombRecipe> recipes_;
  std::vector<const RecombRecipe*> filtered_recipes_;
  bool overlay_visible_ = false;
  std::string search_term_;
  std::string result_filter_;
  nlohmann::json recipe_data_;
};

} // namespace exile::modules