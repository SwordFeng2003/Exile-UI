#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace exile::modules {

struct SearchTemplate {
  std::string name;
  std::string query;
  std::string description;
  bool is_button = true;
  int priority = 0;
};

struct SearchCategory {
  std::string name;
  std::string icon;
  std::vector<SearchTemplate> templates;
};

class SearchStrings {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();

  void load_templates();
  void add_template(const SearchTemplate& tmpl);
  void remove_template(const std::string& name);

  void apply_search(const std::string& query);
  void on_search_trigger();
  void on_omnikey_search();

  void set_trigger_keys(const std::string& click_key, const std::string& omnikey);
  void set_wait_delay(int ms) { wait_delay_ms_ = ms; }

private:
  std::vector<SearchCategory> categories_;
  bool overlay_visible_ = false;
  int wait_delay_ms_ = 50;
  std::string click_key_;
  std::string omnikey_;
  int selected_template_ = 0;
  std::string last_query_;
};

} // namespace exile::modules