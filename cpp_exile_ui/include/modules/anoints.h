#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace exile::modules {

struct AnointInfo {
  std::string name;
  std::string effect;
  std::string icon_path;
  std::vector<std::string> oils;
  bool is_notable = false;
};

class Anoints {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void search_by_name(const std::string& query);
  void search_by_oil(const std::string& oil);
  void search_by_effect(const std::string& effect);

  void parse_clipboard_anoint();
  void on_hover_item();

  void load_anoints_database();
  const AnointInfo* find_anoint(const std::string& name) const;

  void set_check_mouse(bool v) { check_mouse_ = v; }
  void set_check_clipboard(bool v) { check_clipboard_ = v; }

private:
  std::unordered_map<std::string, AnointInfo> anoints_;
  std::vector<const AnointInfo*> search_results_;
  std::string search_query_;
  bool overlay_visible_ = false;
  bool check_mouse_ = true;
  bool check_clipboard_ = true;
  bool clipboard_seen_ = false;
};

} // namespace exile::modules