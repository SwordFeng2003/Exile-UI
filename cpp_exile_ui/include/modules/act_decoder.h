#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <Windows.h>

namespace exile::modules {

struct ZoneLayout {
  std::string zone_id;
  std::string image_path;
  int variant_count = 0;
  int current_variant = 0;
  float rotation = 0.0f;
  bool flipped = false;
};

class ActDecoder {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void toggle_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void load_zone_layouts();
  void match_current_layout();
  void cycle_variant(int direction);

  void set_layout_scale(float scale);
  void set_layout_transparency(float trans);
  void lock_layouts(bool lock);

  void on_area_change(const std::string& area_id);

private:
  std::unordered_map<std::string, ZoneLayout> zone_layouts_;
  std::string current_zone_;
  ZoneLayout* current_layout_ = nullptr;
  bool overlay_visible_ = false;
  bool layouts_locked_ = false;
  float layout_scale_ = 1.0f;
  float layout_transparency_ = 1.0f;
  HBITMAP current_bitmap_ = nullptr;
};

} // namespace exile::modules