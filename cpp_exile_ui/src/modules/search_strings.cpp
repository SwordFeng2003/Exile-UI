#include "modules/search_strings.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/input_simulator.h"
#include "game/clipboard_handler.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>

namespace exile::modules {

void SearchStrings::initialize() {
  load_templates();

  auto& cfg = core::Application::instance().config_manager();
  click_key_ = cfg.read<std::string>("Search-strings", "click-key", "LButton");
  omnikey_ = cfg.read<std::string>("Search-strings", "omnikey", "");
  wait_delay_ms_ = cfg.read<int>("Search-strings", "wait-delay", 50);
}

void SearchStrings::shutdown() {
}

void SearchStrings::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("search strings").c_str(), &overlay_visible_);

  for (auto& category : categories_) {
    if (category.templates.empty()) continue;

    if (ImGui::CollapsingHeader(category.name.c_str())) {
      for (auto& tmpl : category.templates) {
        if (tmpl.is_button) {
          if (ImGui::Button(tmpl.name.c_str())) {
            apply_search(tmpl.query);
          }
          ImGui::SameLine();
          ImGui::Text("%s", tmpl.description.c_str());
        } else {
          ImGui::BulletText("%s - %s", tmpl.name.c_str(), tmpl.query.c_str());
        }
      }
    }
  }

  ImGui::End();
}

void SearchStrings::show_overlay() { overlay_visible_ = true; }
void SearchStrings::hide_overlay() { overlay_visible_ = false; }

void SearchStrings::load_templates() {
  categories_.clear();

  auto add_category = [this](const std::string& name, const std::string& icon,
                              std::vector<SearchTemplate> templates) {
    SearchCategory cat;
    cat.name = name;
    cat.icon = icon;
    cat.templates = std::move(templates);
    categories_.push_back(std::move(cat));
  };

  add_category("Currency", "", {
    {"Chaos Orb", "chaos orb", "Search for Chaos Orb"},
    {"Divine Orb", "divine orb", "Search for Divine Orb"},
    {"Exalted Orb", "exalted orb", "Search for Exalted Orb"},
    {"Mirror", "mirror", "Search for Mirror of Kalandra"},
  });

  add_category("Maps", "", {
    {"Tier 16", "tier: 16", "Search for Tier 16 maps"},
    {"Guardian", "guardian", "Search for Guardian maps"},
    {"Shaper", "shaper", "Search for Shaper Guardian maps"},
    {"Elder", "elder", "Search for Elder Guardian maps"},
  });

  add_category("Equipment", "", {
    {"6-Link", "links: 6", "Search for 6-linked items"},
    {"5-Link", "links: 5", "Search for 5-linked items"},
    {"Enchanted", "enchanted", "Search for enchanted items"},
    {"Fractured", "fractured", "Search for fractured items"},
  });

  add_category("Gems", "", {
    {"20/20", "quality: 20", "Search for 20% quality gems"},
    {"21/20", "quality: 20 level: 21", "Search for 21/20 gems"},
    {"Awakened", "awakened", "Search for Awakened gems"},
    {"Enlighten Lvl 4", "enlighten level: 4", "Search for Enlighten Lvl 4"},
  });
}

void SearchStrings::add_template(const SearchTemplate& tmpl) {
  if (!categories_.empty()) {
    categories_.back().templates.push_back(tmpl);
  }
}

void SearchStrings::remove_template(const std::string& name) {
  for (auto& cat : categories_) {
    cat.templates.erase(
      std::remove_if(cat.templates.begin(), cat.templates.end(),
        [&](const SearchTemplate& t) { return t.name == name; }),
      cat.templates.end());
  }
}

void SearchStrings::apply_search(const std::string& query) {
  last_query_ = query;

  game::InputSimulator::send_ctrl_a();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  game::InputSimulator::send_ctrl_c();
  std::this_thread::sleep_for(std::chrono::milliseconds(wait_delay_ms_));

  game::InputSimulator::send_text(query);

  std::this_thread::sleep_for(std::chrono::milliseconds(wait_delay_ms_));
  game::InputSimulator::send_enter();
}

void SearchStrings::on_search_trigger() {
  apply_search(last_query_);
}

void SearchStrings::on_omnikey_search() {
}

void SearchStrings::set_trigger_keys(const std::string& click_key, const std::string& omnikey) {
  click_key_ = click_key;
  omnikey_ = omnikey;
}

} // namespace exile::modules