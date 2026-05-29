#include "modules/act_decoder.h"
#include "modules/search_strings.h"
#include "core/config_manager.h"
#include "core/application.h"
#include <imgui.h>

namespace exile::modules {

void ActDecoder::initialize() {
  load_zone_layouts();
}

void ActDecoder::shutdown() {
  if (current_bitmap_) DeleteObject(current_bitmap_);
}

void ActDecoder::render_overlay() {
  if (!overlay_visible_ || !current_layout_) return;

  ImDrawList* draw_list = ImGui::GetForegroundDrawList();

  auto& cfg = core::Application::instance().config_manager();
  int x = cfg.read<int>("Act-decoder", "pos_x", 0);
  int y = cfg.read<int>("Act-decoder", "pos_y", 0);
  int w = cfg.read<int>("Act-decoder", "width", 400);
  int h = cfg.read<int>("Act-decoder", "height", 300);

  ImVec2 min(static_cast<float>(x), static_cast<float>(y));
  ImVec2 max(min.x + w * layout_scale_, min.y + h * layout_scale_);

  draw_list->AddRectFilled(min, max, IM_COL32(0, 0, 0, static_cast<int>(200 * layout_transparency_)));

  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(ImVec2(w * layout_scale_, h * layout_scale_));

  ImGui::Begin("##act_decoder_overlay", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
               ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
               ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing);

  if (current_layout_) {
    ImGui::Text("Zone: %s (Variant %d/%d)",
      current_layout_->zone_id.c_str(),
      current_layout_->current_variant + 1,
      current_layout_->variant_count);
    ImGui::Text("Layout: %s", layouts_locked_ ? "Locked" : "Auto");
  }

  ImGui::End();
}

void ActDecoder::toggle_overlay() {
  overlay_visible_ = !overlay_visible_;
}

void ActDecoder::hide_overlay() {
  overlay_visible_ = false;
}

void ActDecoder::load_zone_layouts() {
  auto& cfg = core::Application::instance().config_manager();

  zone_layouts_.clear();
}

void ActDecoder::match_current_layout() {
  if (layouts_locked_) return;
}

void ActDecoder::cycle_variant(int direction) {
  if (!current_layout_) return;
  current_layout_->current_variant += direction;
  if (current_layout_->current_variant < 0)
    current_layout_->current_variant = current_layout_->variant_count - 1;
  if (current_layout_->current_variant >= current_layout_->variant_count)
    current_layout_->current_variant = 0;
}

void ActDecoder::set_layout_scale(float scale) {
  layout_scale_ = scale;
}

void ActDecoder::set_layout_transparency(float trans) {
  layout_transparency_ = trans;
}

void ActDecoder::lock_layouts(bool lock) {
  layouts_locked_ = lock;
}

void ActDecoder::on_area_change(const std::string& area_id) {
  current_zone_ = area_id;
  auto it = zone_layouts_.find(area_id);
  if (it != zone_layouts_.end()) {
    current_layout_ = &it->second;
    match_current_layout();
  }
}

} // namespace exile::modules