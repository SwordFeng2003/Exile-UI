#include "modules/recombination.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/clipboard_handler.h"
#include "ui/widgets.h"
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace exile::modules {

void Recombination::initialize() {
  load_database();
}

void Recombination::shutdown() {
}

void Recombination::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("recombination").c_str(), &overlay_visible_);

  static char search_buf[128] = {};
  if (ImGui::InputText(lang.translate("search").c_str(), search_buf, sizeof(search_buf))) {
    search_recipes(std::string(search_buf));
  }

  ImGui::Separator();

  if (ImGui::BeginTable("##recomb_table", 3,
      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
    ImGui::TableSetupColumn(lang.translate("result").c_str());
    ImGui::TableSetupColumn(lang.translate("ingredients").c_str());
    ImGui::TableSetupColumn(lang.translate("type").c_str());
    ImGui::TableHeadersRow();

    const auto& display = filtered_recipes_.empty() ?
      std::vector<const RecombRecipe*>() : filtered_recipes_;

    for (const auto* recipe : display) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%s", recipe->result.c_str());

      ImGui::TableSetColumnIndex(1);
      std::string ingredients;
      for (size_t i = 0; i < recipe->ingredients.size(); ++i) {
        if (i > 0) ingredients += " + ";
        ingredients += recipe->ingredients[i];
      }
      ImGui::Text("%s", ingredients.c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%s", recipe->crafted ? "Crafted" : "Vendor");
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void Recombination::show_overlay() { overlay_visible_ = true; }
void Recombination::hide_overlay() { overlay_visible_ = false; }

void Recombination::search_recipes(const std::string& query) {
  search_term_ = query;
  filtered_recipes_.clear();

  std::string lower_query = query;
  for (auto& c : lower_query) c = std::tolower(c);

  for (auto& recipe : recipes_) {
    std::string lower_result = recipe.result;
    for (auto& c : lower_result) c = std::tolower(c);

    if (lower_result.find(lower_query) != std::string::npos) {
      filtered_recipes_.push_back(&recipe);
      continue;
    }

    for (auto& ing : recipe.ingredients) {
      std::string lower_ing = ing;
      for (auto& c : lower_ing) c = std::tolower(c);
      if (lower_ing.find(lower_query) != std::string::npos) {
        filtered_recipes_.push_back(&recipe);
        break;
      }
    }
  }
}

void Recombination::filter_by_result(const std::string& result_type) {
  result_filter_ = result_type;
}

void Recombination::parse_clipboard() {
}

void Recombination::on_item_hover() {
}

void Recombination::control_recipes() {
}

void Recombination::check_validity() {
}

const RecombRecipe* Recombination::find_recipe(const std::string& name) const {
  for (auto& recipe : recipes_) {
    if (recipe.name == name || recipe.result == name) return &recipe;
  }
  return nullptr;
}

void Recombination::load_database() {
  auto& res = core::Application::instance().resource_manager();

  if (res.has_json("recombination")) {
    try {
      auto data = nlohmann::json::parse(res.get_json_string("recombination"));
      for (auto& item : data) {
        RecombRecipe recipe;
        recipe.name = item.value("name", "");
        recipe.result = item.value("result", "");
        recipe.description = item.value("description", "");
        recipe.crafted = item.value("crafted", false);

        if (item.contains("ingredients")) {
          for (auto& ing : item["ingredients"]) {
            recipe.ingredients.push_back(ing.get<std::string>());
          }
        }

        recipes_.push_back(recipe);
      }
    } catch (const std::exception&) {}
  }
}

} // namespace exile::modules