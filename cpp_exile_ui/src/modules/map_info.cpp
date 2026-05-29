#include "modules/map_info.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "core/resource_manager.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace exile::modules {

void MapInfo::initialize() {
  load_database();
  load_recipes();
  load_harvest_crafts();

  auto& cfg = core::Application::instance().config_manager();
  window_x_ = cfg.read<int>("Map-info", "pos-x", 100);
  window_y_ = cfg.read<int>("Map-info", "pos-y", 100);
}

void MapInfo::shutdown() {
}

void MapInfo::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window_x_), static_cast<float>(window_y_)),
                          ImGuiCond_FirstUseEver);
  ImGui::Begin(lang.translate("map info").c_str(), &overlay_visible_);

  if (ImGui::BeginTabBar("##map_info_tabs")) {
    if (ImGui::BeginTabItem(lang.translate("map mods").c_str())) {
      static char search_buf[128] = {};
      ImGui::InputText(lang.translate("search mod").c_str(), search_buf, sizeof(search_buf));

      if (ImGui::BeginTable("##mods_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(lang.translate("name").c_str());
        ImGui::TableSetupColumn(lang.translate("description").c_str());
        ImGui::TableSetupColumn(lang.translate("tags").c_str());
        ImGui::TableSetupColumn("Diff.");
        ImGui::TableHeadersRow();

        for (auto& [name, mod] : mods_) {
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%s", mod.name.c_str());
          ImGui::TableSetColumnIndex(1);
          ImGui::TextWrapped("%s", mod.description.c_str());
          ImGui::TableSetColumnIndex(2);
          std::string tags;
          for (auto& t : mod.tags) tags += t + " ";
          ImGui::Text("%s", tags.c_str());
          ImGui::TableSetColumnIndex(3);
          ImGui::Text("%.1f", mod.difficulty);
        }

        ImGui::EndTable();
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem(lang.translate("recipes").c_str())) {
      for (auto& recipe : recipes_) {
        if (ImGui::TreeNode(recipe.name.c_str())) {
          ImGui::TextWrapped("%s", recipe.effect.c_str());
          ImGui::Text("Cost: %s", recipe.cost.c_str());
          ImGui::Text("Location: %s", recipe.location.c_str());
          ImGui::TreePop();
        }
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem(lang.translate("harvest").c_str())) {
      for (auto& craft : harvest_crafts_) {
        if (ImGui::TreeNode(craft.name.c_str())) {
          ImGui::TextWrapped("%s", craft.description.c_str());
          ImGui::Text("Cost: %s | Category: %s", craft.cost.c_str(), craft.category.c_str());
          ImGui::TreePop();
        }
      }
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  ImGui::End();
}

void MapInfo::show_overlay() { overlay_visible_ = true; }
void MapInfo::hide_overlay() { overlay_visible_ = false; }

void MapInfo::search_map(const std::string& query) {
  search_query_ = query;
}

void MapInfo::search_mod(const std::string& query) {
  search_query_ = query;
}

void MapInfo::search_recipe(const std::string& query) {
  search_query_ = query;
}

const MapModInfo* MapInfo::get_mod(const std::string& name) const {
  auto it = mods_.find(name);
  if (it != mods_.end()) return &it->second;
  return nullptr;
}

const MapBaseInfo* MapInfo::get_map(const std::string& name) const {
  auto it = maps_.find(name);
  if (it != maps_.end()) return &it->second;
  return nullptr;
}

void MapInfo::load_database() {
  auto& res = core::Application::instance().resource_manager();
  if (!res.has_json("map_mods")) return;

  try {
    auto data = nlohmann::json::parse(res.get_json_string("map_mods"));
    for (auto& item : data) {
      MapModInfo mod;
      mod.name = item.value("name", "");
      mod.description = item.value("description", "");
      mod.difficulty = item.value("difficulty", 0.0f);
      mod.reward_value = item.value("reward", 0.0f);

      if (item.contains("tags")) {
        for (auto& tag : item["tags"]) {
          mod.tags.push_back(tag.get<std::string>());
        }
      }

      mods_[mod.name] = mod;
    }
  } catch (const std::exception&) {}
}

void MapInfo::load_recipes() {
  auto& res = core::Application::instance().resource_manager();
  if (!res.has_json("crafting_recipes")) return;

  try {
    auto data = nlohmann::json::parse(res.get_json_string("crafting_recipes"));
    for (auto& item : data) {
      CraftingRecipe recipe;
      recipe.name = item.value("name", "");
      recipe.effect = item.value("effect", "");
      recipe.cost = item.value("cost", "");
      recipe.location = item.value("location", "");
      recipes_.push_back(recipe);
    }
  } catch (const std::exception&) {}
}

void MapInfo::load_harvest_crafts() {
  auto& res = core::Application::instance().resource_manager();
  if (!res.has_json("harvest_crafts")) return;

  try {
    auto data = nlohmann::json::parse(res.get_json_string("harvest_crafts"));
    for (auto& item : data) {
      HarvestCraft craft;
      craft.name = item.value("name", "");
      craft.category = item.value("category", "");
      craft.description = item.value("description", "");
      craft.cost = item.value("cost", "");
      harvest_crafts_.push_back(craft);
    }
  } catch (const std::exception&) {}
}

void MapInfo::check_map_mods() {
}

} // namespace exile::modules