#include "modules/item_checker.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/clipboard_handler.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <regex>
#include <algorithm>

namespace exile::modules {

void ItemChecker::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  activation_mode_ = cfg.read<std::string>("Item-checker", "activation-mode", "toggle");
  compare_enabled_ = cfg.read<bool>("Item-checker", "compare-enabled", false);
}

void ItemChecker::shutdown() {
  close_tooltip();
}

bool ItemChecker::parse_clipboard_item() {
  auto text = game::ClipboardHandler::instance().get_text();
  if (text.empty()) return false;

  current_item_ = ItemInfo{};
  highlighted_stats_.clear();

  std::vector<std::string> lines;
  std::string line;
  for (char c : text) {
    if (c == '\n') {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (!line.empty()) {
        lines.push_back(line);
        line.clear();
      }
    } else {
      line += c;
    }
  }
  if (!line.empty()) lines.push_back(line);

  if (lines.empty()) return false;

  for (size_t i = 0; i < lines.size(); ++i) {
    auto& l = lines[i];

    if (l.find("Rarity:") != std::string::npos) {
      current_item_.rarity = l.substr(l.find(':') + 2);
    } else if (l.find("Item Class:") != std::string::npos) {
      current_item_.item_class = l.substr(l.find(':') + 2);
    } else if (l.find("Item Level:") != std::string::npos) {
      current_item_.item_level = std::stoi(l.substr(l.find(':') + 2));
    } else if (l.find("Level:") != std::string::npos) {
      current_item_.required_level = std::stoi(l.substr(l.find(':') + 2));
    } else if (l.find("Quality:") != std::string::npos) {
    } else if (l.find("Sockets:") != std::string::npos) {
    } else if (l.find("Corrupted") != std::string::npos) {
      current_item_.corrupted = true;
    } else if (l.find("Item Level: ") != std::string::npos) {
    } else if (l.find("Map Tier:") != std::string::npos) {
      current_item_.map_tier = std::stoi(l.substr(l.find(':') + 2));
    } else if (i == 1) {
      current_item_.name = l;
    } else if (l.find("--------") != std::string::npos) {
      break;
    }
  }

  tooltip_visible_ = true;
  return true;
}

void ItemChecker::render_tooltip() {
  if (!tooltip_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::SetNextWindowPos(ImVec2(static_cast<float>(tooltip_x_), static_cast<float>(tooltip_y_)),
                          ImGuiCond_Appearing);
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(tooltip_w_), 0), ImGuiCond_Always);

  ImGui::Begin(lang.translate("item tooltip").c_str(), &tooltip_visible_,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::TextWrapped("%s", current_item_.name.c_str());
  ImGui::Separator();

  ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", current_item_.rarity.c_str());
  ImGui::SameLine();
  ImGui::Text("%s", current_item_.item_class.c_str());

  if (current_item_.item_level > 0) {
    ImGui::Text("Item Level: %d", current_item_.item_level);
  }
  if (current_item_.required_level > 0) {
    ImGui::Text("Required Level: %d", current_item_.required_level);
  }
  if (current_item_.map_tier > 0) {
    ImGui::Text("Map Tier: %d", current_item_.map_tier);
  }
  if (current_item_.corrupted) {
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Corrupted");
  }

  for (auto& imp : current_item_.implicits) {
    ImGui::Text("%s: %s", imp.name.c_str(), imp.value.c_str());
  }

  ImGui::End();
}

void ItemChecker::close_tooltip() {
  tooltip_visible_ = false;
}

void ItemChecker::highlight_stat(const std::string& stat_name) {
  highlighted_stats_[stat_name] = 1;
}

void ItemChecker::clear_highlights() {
  highlighted_stats_.clear();
}

void ItemChecker::handle_gear_parse() {
  auto text = game::ClipboardHandler::instance().get_text();
  if (text.empty()) return;

  if (text.find("Item Class:") != std::string::npos) {
    parse_clipboard_item();
  }
}

void ItemChecker::update_gear_slot(const std::string& slot) {
}

void ItemChecker::parse_item_name(const std::string& line) {
  current_item_.name = line;
}

void ItemChecker::parse_item_stats(const std::string& line) {
  ItemStat stat;
  auto delim = line.find(':');
  if (delim != std::string::npos) {
    stat.name = line.substr(0, delim);
    stat.value = line.substr(delim + 1);
  }
}

void ItemChecker::parse_requirements(const std::string& line) {
}

void ItemChecker::determine_tiers() {
}

} // namespace exile::modules