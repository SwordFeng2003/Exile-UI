#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace exile::modules {

struct SanctumRoom {
  std::string id;
  std::string name;
  std::string type;
  int row = 0;
  int col = 0;
  int weight = 0;
  bool revealed = false;
  bool completed = false;
  bool marked = false;
};

struct SanctumFloor {
  int floor_number = 0;
  std::vector<std::vector<SanctumRoom>> grid;
  int row_count = 0;
  int col_count = 0;
};

struct SanctumRelic {
  std::string name;
  std::string modifier;
  int quantity = 0;
  int honor_resistance = 0;
  int max_honor = 0;
  int inspiration = 0;
};

class SanctumPlanner {
public:
  void initialize();
  void shutdown();

  void render_main_window();
  void render_floor_view();
  void render_relics_window();

  void scan_floor();
  void parse_floor_image();
  void plan_optimal_path();

  void mark_room(int row, int col, int mode);
  void toggle_room_reveal(int row, int col);

  void load_relics();
  void save_relics();
  void add_relic(const SanctumRelic& relic);
  void remove_relic(size_t index);

  void toggle_lock();
  bool is_locked() const { return locked_; }
  bool is_visible() const { return visible_; }
  void set_visible(bool v) { visible_ = v; }
  bool is_scanning() const { return scanning_; }

private:
  std::vector<SanctumFloor> floors_;
  int current_floor_ = 0;
  std::vector<SanctumRelic> relics_;
  bool visible_ = false;
  bool locked_ = false;
  bool scanning_ = false;
  float zoom_ = 1.0f;
};

} // namespace exile::modules