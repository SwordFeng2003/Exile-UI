#include "modules/anoints.h"
#include "core/config_manager.h"
#include "core/application.h"
#include "game/clipboard_handler.h"
#include "ui/widgets.h"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace exile::modules {

void Anoints::initialize() {
  load_anoints_database();

  auto& cfg = core::Application::instance().config_manager();
  check_mouse_ = cfg.read<bool>("Anoints", "check-mouse", true);
  check_clipboard_ = cfg.read<bool>("Anoints", "check-clipboard", true);
}

void Anoints::shutdown() {
}

void Anoints::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("anoints").c_str(), &overlay_visible_);

  static char search_buf[128] = {};
  ImGui::InputText(lang.translate("search").c_str(), search_buf, sizeof(search_buf));
  if (search_buf[0] != '\0') {
    search_by_name(std::string(search_buf));
  }

  ImGui::Separator();

  if (ImGui::BeginTable("##anoints_table", 3,
      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
    ImGui::TableSetupColumn(lang.translate("name").c_str());
    ImGui::TableSetupColumn(lang.translate("oils").c_str());
    ImGui::TableSetupColumn(lang.translate("effect").c_str());
    ImGui::TableHeadersRow();

    const auto& display = search_results_.empty() ?
      std::vector<const AnointInfo*>() : search_results_;

    for (const auto& info : display) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%s", info->name.c_str());

      ImGui::TableSetColumnIndex(1);
      std::string oils_str;
      for (auto& o : info->oils) oils_str += o + " ";
      ImGui::Text("%s", oils_str.c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::TextWrapped("%s", info->effect.c_str());
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void Anoints::show_overlay() { overlay_visible_ = true; }
void Anoints::hide_overlay() { overlay_visible_ = false; }

void Anoints::search_by_name(const std::string& query) {
  search_query_ = query;
  search_results_.clear();

  std::string lower_query = query;
  for (auto& c : lower_query) c = std::tolower(c);

  for (auto& [name, info] : anoints_) {
    std::string lower_name = name;
    for (auto& c : lower_name) c = std::tolower(c);
    if (lower_name.find(lower_query) != std::string::npos) {
      search_results_.push_back(&info);
    }
  }
}

void Anoints::search_by_oil(const std::string& oil) {
  search_results_.clear();
  for (auto& [name, info] : anoints_) {
    for (auto& o : info.oils) {
      if (o == oil) {
        search_results_.push_back(&info);
        break;
      }
    }
  }
}

void Anoints::search_by_effect(const std::string& effect) {
  search_results_.clear();
  for (auto& [name, info] : anoints_) {
    if (info.effect.find(effect) != std::string::npos) {
      search_results_.push_back(&info);
    }
  }
}

void Anoints::parse_clipboard_anoint() {
  if (!check_clipboard_) return;

  auto text = game::ClipboardHandler::instance().get_text();
  if (text.empty() || text == clipboard_seen_ ? text : "") return;

  clipboard_seen_ = text;

  for (auto& [name, info] : anoints_) {
    if (text.find(name) != std::string::npos) {
      search_by_name(name);
      show_overlay();
      return;
    }
  }
}

void Anoints::on_hover_item() {
  if (!check_mouse_ || !overlay_visible_) return;
}

void Anoints::load_anoints_database() {
  auto& res = core::Application::instance().resource_manager();

  if (res.has_json("anoints")) {
    auto data = nlohmann::json::parse(res.get_json_string("anoints"));
    for (auto& item : data) {
      AnointInfo info;
      info.name = item.value("name", "");
      info.effect = item.value("effect", "");
      info.is_notable = item.value("notable", false);

      if (item.contains("oils")) {
        for (auto& oil : item["oils"]) {
          info.oils.push_back(oil.get<std::string>());
        }
      }

      anoints_[info.name] = info;
    }
  }
}

const AnointInfo* Anoints::find_anoint(const std::string& name) const {
  auto it = anoints_.find(name);
  if (it != anoints_.end()) return &it->second;
  return nullptr;
}

} // namespace exile::modules