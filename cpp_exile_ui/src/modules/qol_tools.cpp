#include "modules/qol_tools.h"
#include "core/config_manager.h"
#include "core/application.h"
#include "game/input_simulator.h"
#include "game/log_parser.h"
#include <imgui.h>
#include <thread>

namespace exile::modules {

void QolTools::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  blight_auto_tower_ = cfg.read<bool>("Qol", "blight-auto-tower", false);
  auto_flask_ = cfg.read<bool>("Qol", "auto-flask", false);
  auto_level_gem_ = cfg.read<bool>("Qol", "auto-level-gem", false);
  auto_quality_gem_ = cfg.read<bool>("Qol", "auto-quality-gem", false);
  auto_id_ = cfg.read<bool>("Qol", "auto-id", false);

  life_threshold_ = cfg.read<float>("Qol", "life-threshold", 0.5f);
  es_threshold_ = cfg.read<float>("Qol", "es-threshold", 0.5f);
  mana_threshold_ = cfg.read<float>("Qol", "mana-threshold", 0.3f);
}

void QolTools::shutdown() {
}

void QolTools::render_panel() {
  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("qol tools").c_str(), nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::SeparatorText(lang.translate("automation").c_str());

  ImGui::Checkbox(lang.translate("auto flask").c_str(), &auto_flask_);
  ImGui::Checkbox(lang.translate("auto level gem").c_str(), &auto_level_gem_);
  ImGui::Checkbox(lang.translate("auto quality gem").c_str(), &auto_quality_gem_);
  ImGui::Checkbox(lang.translate("auto ID items").c_str(), &auto_id_);

  ImGui::SeparatorText(lang.translate("thresholds").c_str());
  ImGui::SliderFloat(lang.translate("life threshold").c_str(), &life_threshold_, 0.0f, 1.0f);
  ImGui::SliderFloat(lang.translate("mana threshold").c_str(), &mana_threshold_, 0.0f, 1.0f);
  ImGui::SliderFloat(lang.translate("ES threshold").c_str(), &es_threshold_, 0.0f, 1.0f);

  ImGui::End();
}

void QolTools::on_omnikey(const std::string& key) {
  if (key == "f1") {
    boss_toggle_ = !boss_toggle_;
  } else if (key == "f2") {
    keyboard_blocked_ = !keyboard_blocked_;
  } else if (key == "f3") {
    mouse_blocked_ = !mouse_blocked_;
  } else if (key == "1") { send_flask(1); }
  else if (key == "2") { send_flask(2); }
  else if (key == "3") { send_flask(3); }
  else if (key == "4") { send_flask(4); }
  else if (key == "5") { send_flask(5); }
}

void QolTools::check_life() {
}

void QolTools::check_es() {
}

void QolTools::check_mana() {
}

void QolTools::check_flask_slots() {
}

void QolTools::set_flask_threshold(int n, float threshold) {
  if (n >= 1 && n <= 5) flask_thresholds_[n - 1] = threshold;
}

void QolTools::set_life_threshold(float threshold) { life_threshold_ = threshold; }
void QolTools::set_es_threshold(float threshold) { es_threshold_ = threshold; }
void QolTools::set_mana_threshold(float threshold) { mana_threshold_ = threshold; }

void QolTools::read_chat_commands() {
}

void QolTools::process_commands(const std::string& line) {
}

void QolTools::execute_command(const std::string& cmd, const std::string& args) {
}

void QolTools::send_flask(int slot) {
  uint32_t key = '0' + slot;
  game::InputSimulator::send_key_press(key, 20);
}

} // namespace exile::modules