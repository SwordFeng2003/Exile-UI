#include "modules/stash_ninja.h"
#include "core/config_manager.h"
#include "core/application.h"
#include "game/economy_fetcher.h"
#include "game/clipboard_handler.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <fstream>

namespace exile::modules {

void StashNinja::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  stash_width_ = cfg.read<int>("Stash-ninja", "stash-width", 600);
  active_league_ = cfg.read<std::string>("Stash-ninja", "league", "Standard");

  load_cached_prices("currency");
}

void StashNinja::shutdown() {
}

void StashNinja::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(stash_width_), 400), ImGuiCond_FirstUseEver);
  ImGui::Begin(lang.translate("stash ninja").c_str(), &overlay_visible_,
               ImGuiWindowFlags_NoSavedSettings);

  ImGui::Text("%s: %s", lang.translate("active league").c_str(), active_league_.c_str());
  ImGui::SameLine();

  if (ImGui::Button(lang.translate("update prices").c_str())) {
    update_prices(active_league_);
  }

  ImGui::Separator();

  if (ImGui::BeginTable("##stash_table", 4,
      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
      ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
    ImGui::TableSetupColumn(lang.translate("item name").c_str());
    ImGui::TableSetupColumn("Chaos", ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn("Divine", ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn(lang.translate("trend").c_str(), ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableHeadersRow();

    int i = 0;
    for (auto& [name, item] : items_) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%s", name.c_str());

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%.1f", item.chaos_value);

      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%.4f", item.divine_value);

      ImGui::TableSetColumnIndex(3);
      if (item.trend > 0) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "+%.1f%%", item.trend);
      } else if (item.trend < 0) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%.1f%%", item.trend);
      } else {
        ImGui::Text("-");
      }

      i++;
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void StashNinja::show_overlay() {
  overlay_visible_ = true;
}

void StashNinja::hide_overlay() {
  overlay_visible_ = false;
}

void StashNinja::close_overlay() {
  overlay_visible_ = false;
}

void StashNinja::update_prices(const std::string& league) {
  if (needs_price_update()) {
    fetch_prices("currency");
    mark_price_updated();
  }
}

bool StashNinja::fetch_prices(const std::string& category) {
  game::EconomyFetcher fetcher;
  if (!fetcher.fetch_currency_prices(active_league_)) return false;

  const auto& fetched = fetcher.prices();
  for (const auto& [name, price] : fetched) {
    items_[name] = {name, price.chaos_value, price.divine_value, {}};
  }

  return true;
}

bool StashNinja::load_cached_prices(const std::string& category) {
  return true;
}

void StashNinja::save_prices_to_cache(const std::string& category) {
}

const StashItem* StashNinja::find_item(const std::string& name) const {
  auto it = items_.find(name);
  if (it != items_.end()) return &it->second;
  return nullptr;
}

float StashNinja::get_price(const std::string& item_name) const {
  auto* item = find_item(item_name);
  return item ? item->chaos_value : 0.0f;
}

void StashNinja::set_active_tab(int index) {
  active_tab_ = index;
}

void StashNinja::refresh_overlay() {
}

void StashNinja::handle_hover(int mouse_x, int mouse_y) {
}

bool StashNinja::needs_price_update() const {
  auto now = std::chrono::duration_cast<std::chrono::minutes>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  return (now - static_cast<int64_t>(last_update_time_)) > 30;
}

void StashNinja::mark_price_updated() {
  last_update_time_ = static_cast<uint64_t>(
    std::chrono::duration_cast<std::chrono::minutes>(
      std::chrono::system_clock::now().time_since_epoch()).count());
}

} // namespace exile::modules