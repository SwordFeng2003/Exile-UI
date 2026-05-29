#include "modules/clone_frames.h"
#include "core/overlay_manager.h"
#include "core/config_manager.h"
#include "core/application.h"
#include "game/screen_checker.h"
#include <imgui.h>

namespace exile::modules {

void CloneFrames::initialize() {
  auto& cfg = core::Application::instance().config_manager();

  gamescreen_check_ = cfg.read<bool>("Clone-frames", "gamescreen-check", true);
  closebutton_check_ = cfg.read<bool>("Clone-frames", "closebutton-check", false);
  inventory_check_ = cfg.read<bool>("Clone-frames", "inventory-check", false);

  int frame_count = cfg.read<int>("Clone-frames", "count", 0);
  for (int i = 1; i <= frame_count; ++i) {
    CloneFrame frame;
    frame.name = "clone-frame-" + std::to_string(i);
    frame.source_x = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_sx", 0);
    frame.source_y = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_sy", 0);
    frame.source_w = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_sw", 100);
    frame.source_h = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_sh", 100);
    frame.target_x = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_tx", 0);
    frame.target_y = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_ty", 0);
    frame.target_w = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_tw", 100);
    frame.target_h = cfg.read<int>("Clone-frames", "f" + std::to_string(i) + "_th", 100);
    frame.opacity = cfg.read<float>("Clone-frames", "f" + std::to_string(i) + "_opacity", 1.0f);
    frame.enabled = cfg.read<bool>("Clone-frames", "f" + std::to_string(i) + "_enabled", true);

    auto& overlay = core::Application::instance().overlay_manager();
    frame.overlay_hwnd = overlay.create_overlay(
      frame.name, frame.target_x, frame.target_y,
      frame.target_w, frame.target_h
    );

    frames_.push_back(std::move(frame));
  }
}

void CloneFrames::shutdown() {
  for (auto& frame : frames_) {
    core::Application::instance().overlay_manager().destroy_overlay(frame.name);
  }
  frames_.clear();
}

void CloneFrames::add_frame(const CloneFrame& frame) {
  frames_.push_back(frame);
}

void CloneFrames::remove_frame(size_t index) {
  if (index < frames_.size()) {
    core::Application::instance().overlay_manager().destroy_overlay(frames_[index].name);
    frames_.erase(frames_.begin() + index);
  }
}

void CloneFrames::update_frame(size_t index, const CloneFrame& frame) {
  if (index < frames_.size()) {
    frames_[index] = frame;
  }
}

void CloneFrames::start_editing() {
  editing_ = true;
  show_all();
}

void CloneFrames::stop_editing(bool save) {
  editing_ = false;
  if (save) {
    auto& cfg = core::Application::instance().config_manager();
    cfg.write("Clone-frames", "count", static_cast<int>(frames_.size()));
    for (size_t i = 0; i < frames_.size(); ++i) {
      auto& f = frames_[i];
      auto prefix = "f" + std::to_string(i + 1);
      cfg.write("Clone-frames", prefix + "_sx", f.source_x);
      cfg.write("Clone-frames", prefix + "_sy", f.source_y);
      cfg.write("Clone-frames", prefix + "_sw", f.source_w);
      cfg.write("Clone-frames", prefix + "_sh", f.source_h);
      cfg.write("Clone-frames", prefix + "_tx", f.target_x);
      cfg.write("Clone-frames", prefix + "_ty", f.target_y);
      cfg.write("Clone-frames", prefix + "_tw", f.target_w);
      cfg.write("Clone-frames", prefix + "_th", f.target_h);
      cfg.write("Clone-frames", prefix + "_opacity", f.opacity);
      cfg.write("Clone-frames", prefix + "_enabled", f.enabled);
    }
  }
}

void CloneFrames::render_all() {
  for (auto& frame : frames_) {
    if (!frame.enabled) continue;
    if (frame.overlay_hwnd) {
      ShowWindow(frame.overlay_hwnd, SW_SHOWNOACTIVATE);
    }
  }
}

void CloneFrames::render_borders() {
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();
  for (auto& frame : frames_) {
    if (!frame.enabled) continue;
    ImVec2 min(static_cast<float>(frame.target_x), static_cast<float>(frame.target_y));
    ImVec2 max(min.x + frame.target_w, min.y + frame.target_h);
    draw_list->AddRect(min, max, IM_COL32(0, 255, 0, 128));
  }
}

void CloneFrames::capture_frames() {
  frame_counter_++;
}

void CloneFrames::hide_all() {
  for (auto& frame : frames_) {
    ShowWindow(frame.overlay_hwnd, SW_HIDE);
  }
}

void CloneFrames::show_all() {
  for (auto& frame : frames_) {
    ShowWindow(frame.overlay_hwnd, SW_SHOWNOACTIVATE);
  }
}

} // namespace exile::modules