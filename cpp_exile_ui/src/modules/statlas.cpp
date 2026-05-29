#include "modules/statlas.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "core/resource_manager.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>
#include <cmath>
#include <nlohmann/json.hpp>

namespace exile::modules {

void Statlas::initialize() {
  load_atlas_data();
  calculate_orbit_positions();

  auto& cfg = core::Application::instance().config_manager();
  max_points_ = cfg.read<int>("Statlas", "max-points", 132);
  max_travel_nodes_ = cfg.read<int>("Statlas", "max-travel", 6);
  ui_scaling_ = cfg.read<float>("Statlas", "ui-scaling", 1.0f);
  curved_connections_ = cfg.read<bool>("Statlas", "curved-connections", true);
}

void Statlas::shutdown() {
}

void Statlas::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::SetNextWindowSize(ImVec2(800 * ui_scaling_, 600 * ui_scaling_), ImGuiCond_FirstUseEver);
  ImGui::Begin(lang.translate("atlas tree").c_str(), &overlay_visible_,
               ImGuiWindowFlags_NoSavedSettings);

  ImGui::Text("%s: %d/%d", lang.translate("allocated").c_str(), total_allocated(), max_points_);
  ImGui::SameLine();
  if (ImGui::Button(lang.translate("refund all").c_str())) refund_all();

  static char search_buf[128] = {};
  ImGui::InputText(lang.translate("search").c_str(), search_buf, sizeof(search_buf));

  ImGui::Separator();

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size(780 * ui_scaling_, 500 * ui_scaling_);

  float radius = 150.0f * ui_scaling_;
  ImVec2 center(canvas_pos.x + canvas_size.x * 0.5f, canvas_pos.y + canvas_size.y * 0.5f);

  draw_list->AddCircleFilled(center, radius + 20.0f, IM_COL32(20, 20, 20, 255));

  for (auto& [id, node] : nodes_) {
    float angle = static_cast<float>(node.orbit_index) * 2.0f * 3.14159f / 8.0f;
    float r = static_cast<float>(node.orbit + 1) * radius / 4.0f;
    float x = center.x + std::cos(angle) * r;
    float y = center.y + std::sin(angle) * r;

    node.x = static_cast<int>(x);
    node.y = static_cast<int>(y);

    ImU32 color = node.allocated ? IM_COL32(0, 255, 0, 255) : IM_COL32(100, 100, 100, 255);
    if (node.highlight) color = IM_COL32(255, 255, 0, 255);

    draw_list->AddCircleFilled(ImVec2(x, y), 6.0f, color);
    draw_list->AddText(ImVec2(x - 20, y + 8), IM_COL32(200, 200, 200, 255),
                      node.name.c_str());
  }

  ImGui::End();
}

void Statlas::show_overlay() { overlay_visible_ = true; }
void Statlas::hide_overlay() { overlay_visible_ = false; }

void Statlas::load_atlas_data() {
  auto& res = core::Application::instance().resource_manager();

  if (res.has_json("atlas_tree")) {
    try {
      auto data = nlohmann::json::parse(res.get_json_string("atlas_tree"));
      for (auto& item : data) {
        AtlasNode node;
        node.id = item.value("id", "");
        node.name = item.value("name", "");
        node.description = item.value("description", "");
        node.group = item.value("group", 0);
        node.orbit = item.value("orbit", 0);
        node.orbit_index = item.value("orbit_index", 0);

        nodes_[node.id] = node;
      }
    } catch (const std::exception&) {}
  }
}

void Statlas::calculate_orbit_positions() {
  for (auto& [id, node] : nodes_) {
    node.orbit_index = node.orbit_index % 8;
  }
}

void Statlas::allocate_node(const std::string& node_id) {
  auto it = nodes_.find(node_id);
  if (it != nodes_.end() && total_allocated() < max_points_) {
    it->second.allocated = true;
  }
}

void Statlas::deallocate_node(const std::string& node_id) {
  auto it = nodes_.find(node_id);
  if (it != nodes_.end()) {
    it->second.allocated = false;
  }
}

void Statlas::refund_all() {
  for (auto& [id, node] : nodes_) {
    node.allocated = false;
  }
}

int Statlas::total_allocated() const {
  int count = 0;
  for (auto& [id, node] : nodes_) {
    if (node.allocated) count++;
  }
  return count;
}

void Statlas::build_tree_view() {
  tree_roots_.clear();
}

float Statlas::sum_circle(int start, int count) const {
  return static_cast<float>(count) * 2.0f * 3.14159f;
}

void Statlas::on_tree_search(const std::string& query) {
  for (auto& [id, node] : nodes_) {
    std::string lower_name = node.name;
    std::string lower_query = query;
    for (auto& c : lower_name) c = std::tolower(c);
    for (auto& c : lower_query) c = std::tolower(c);
    node.highlight = !query.empty() && lower_name.find(lower_query) != std::string::npos;
  }
}

} // namespace exile::modules