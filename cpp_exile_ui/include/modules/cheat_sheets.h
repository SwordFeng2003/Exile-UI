#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct CheatSheetEntry {
  std::string label;
  std::string content;
  std::vector<std::string> lines;
  std::string source_file;
  bool is_image = false;
  std::string image_path;
};

struct CheatSheetCategory {
  std::string name;
  std::string source_file;
  std::vector<CheatSheetEntry> entries;
  int current_index = 0;
};

class CheatSheets {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  void close_overlay();

  void load_category(const std::string& category);
  void cycle_entry(int direction);
  void navigate_to_category(int index);

  void set_position(int x, int y);
  void set_scale(float scale);
  void set_opacity(float opacity);

  bool is_visible() const { return overlay_visible_; }

private:
  std::vector<CheatSheetCategory> categories_;
  int current_category_ = 0;
  bool overlay_visible_ = false;
  int pos_x_ = 0, pos_y_ = 0;
  float scale_ = 1.0f;
  float opacity_ = 1.0f;
  std::string search_term_;
  bool lock_position_ = false;

  nlohmann::json visuals_config_;
};

} // namespace exile::modules