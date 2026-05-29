#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace exile::modules {

struct BetrayalMember {
  std::string name;
  int division = 0;
  int trust = 0;
  int rank = 0;
  std::string reward;
  bool locked = false;
  int target_division = 0;
};

enum class BetrayalDivision {
  Transportation = 0,
  Fortification = 1,
  Research = 2,
  Intervention = 3,
  None = 4
};

class BetrayalInfo {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void load_member_data();
  void parse_betrayal_ui();
  void update_member(int index, const BetrayalMember& member);
  void execute_action(const std::string& action);

  void highlight_rewards();
  BetrayalMember* find_member(const std::string& name);

  void set_custom_background(bool v) { custom_bg_ = v; }

private:
  std::vector<BetrayalMember> members_;
  std::vector<std::string> rewards_;
  bool overlay_visible_ = false;
  bool custom_bg_ = false;
  std::unordered_map<std::string, std::string> reward_descriptions_;
};

} // namespace exile::modules