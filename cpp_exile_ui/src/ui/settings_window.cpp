#include "ui/settings_window.h"
#include "ui/theme.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace exile::ui {

void SettingsWindow::register_tab(const SettingsTab& tab) {
  tabs_.push_back(tab);
}

void SettingsWindow::show() {
  visible_ = true;
}

void SettingsWindow::hide() {
  visible_ = false;
}

void SettingsWindow::set_active_tab(const std::string& name) {
  auto& cfg = core::Application::instance().config_manager();
  cfg.write("Settings", "tab-startup", name);
  active_tab_ = name;
}

void SettingsWindow::render() {
  if (!visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window_width_), 600.0f), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window_x_), static_cast<float>(window_y_)),
                          ImGuiCond_FirstUseEver);

  ImGui::Begin(lang.translate("settings title").c_str(), &visible_,
               ImGuiWindowFlags_NoDocking);

  render_tab_bar();

  ImGui::Columns(2, "settings_columns", false);
  ImGui::SetColumnWidth(0, static_cast<float>(selection_width_));

  render_tab_content();

  ImGui::NextColumn();

  auto it = std::find_if(tabs_.begin(), tabs_.end(),
      [this](const SettingsTab& t) { return t.name == active_tab_; });
  if (it != tabs_.end() && it->render_func) {
    it->render_func();
  }

  ImGui::Columns(1);
  ImGui::End();
}

void SettingsWindow::render_tab_bar() {
  auto& lang = core::Application::instance().language_manager();

  for (auto& tab : tabs_) {
    bool selected = (tab.name == active_tab_);
    if (ImGui::Selectable(tab.label.c_str(), selected)) {
      set_active_tab(tab.name);
    }
    if (ImGui::IsItemHovered() && tab.is_beta) {
      ImGui::SetTooltip("(%s)", lang.translate("beta feature").c_str());
    }
  }
}

void SettingsWindow::render_tab_content() {
}

void SettingsWindow::render_general_settings() {
  auto& lang = core::Application::instance().language_manager();
  auto& cfg = core::Application::instance().config_manager();

  ImGui::SeparatorText(lang.translate("general settings").c_str());

  static char omnikey_buf[64] = {};
  ImGui::InputText(lang.translate("omnikey").c_str(), omnikey_buf, sizeof(omnikey_buf));

  bool dev_mode = cfg.read<bool>("Internal", "dev_mode", false);
  if (ImGui::Checkbox(lang.translate("developer mode").c_str(), &dev_mode)) {
    cfg.write("Internal", "dev_mode", dev_mode);
  }

  int target_fps = cfg.read<int>("General", "fps", 60);
  if (ImGui::SliderInt(lang.translate("target fps").c_str(), &target_fps, 10, 144)) {
    cfg.write("General", "fps", target_fps);
  }

  ImGui::SeparatorText(lang.translate("ui language").c_str());
  const char* languages[] = { "english", "french", "german", "spanish", "portuguese",
                              "russian", "korean", "chinese", "taiwanese" };
  static int current_lang = 0;
  if (ImGui::Combo("##lang_combo", &current_lang, languages, IM_ARRAYSIZE(languages))) {
    cfg.write("English", "active-ui-language", std::string(languages[current_lang]));
  }
}

void SettingsWindow::render_feature_toggles() {
  auto& lang = core::Application::instance().language_manager();
  auto& cfg = core::Application::instance().config_manager();

  ImGui::SeparatorText(lang.translate("feature toggles").c_str());

  struct Feature {
    const char* key;
    const char* label;
    bool is_beta = false;
  };

  Feature features[] = {
    {"clone-frames-enabled", "Clone-frames"},
    {"item-checker-enabled", "Item-checker"},
    {"leveling-tracker-enabled", "Leveling tracker"},
    {"act-decoder-enabled", "Act decoder"},
    {"stash-ninja-enabled", "Stash-ninja"},
    {"sanctum-planner-enabled", "Sanctum planner", true},
    {"lootfilter-enabled", "FilterSpoon"},
    {"map-tracker-enabled", "Map tracker"},
    {"exchange-enabled", "Vaal Street / Exchange"},
    {"anoints-enabled", "Anoints"},
    {"betrayal-info-enabled", "Betrayal info"},
    {"cheat-sheets-enabled", "Cheat sheets"},
    {"macros-enabled", "Macros"},
    {"map-info-enabled", "Map info"},
    {"search-strings-enabled", "Search strings"},
    {"recombination-enabled", "Recombination"},
    {"qol-tools-enabled", "QoL tools"},
    {"seed-explorer-enabled", "Seed explorer"},
    {"statlas-enabled", "Statlas", true},
  };

  for (auto& feat : features) {
    bool enabled = cfg.read<bool>("Features", feat.key, true);
    bool changed = ImGui::Checkbox(feat.label, &enabled);
    if (feat.is_beta) {
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "(beta)");
    }
    if (changed) {
      cfg.write("Features", feat.key, enabled);
    }
  }
}

void SettingsWindow::render_hotkey_settings() {
  auto& lang = core::Application::instance().language_manager();

  ImGui::SeparatorText(lang.translate("hotkeys").c_str());
  ImGui::TextWrapped(lang.translate("hotkeys description").c_str());

  struct HotkeyField {
    const char* key;
    const char* label;
  };

  HotkeyField fields[] = {
    {"hotkey-clone-frames", "Clone-frames toggle"},
    {"hotkey-item-checker", "Item-checker toggle"},
    {"hotkey-leveling-tracker", "Leveling tracker toggle"},
    {"hotkey-act-decoder", "Act-decoder toggle"},
    {"hotkey-map-tracker", "Map tracker toggle"},
    {"hotkey-cheat-sheets", "Cheat sheets toggle"},
    {"hotkey-macros", "Macros"},
  };

  auto& cfg = core::Application::instance().config_manager();
  for (auto& f : fields) {
    static char buf[64] = {};
    auto val = cfg.read<std::string>("Hotkeys", f.key, "");
    strncpy_s(buf, val.c_str(), sizeof(buf) - 1);
    if (ImGui::InputText(f.label, buf, sizeof(buf))) {
      cfg.write("Hotkeys", f.key, std::string(buf));
    }
  }
}

void SettingsWindow::render_screen_check_settings() {
  auto& lang = core::Application::instance().language_manager();
  auto& cfg = core::Application::instance().config_manager();

  ImGui::SeparatorText(lang.translate("screen checks").c_str());

  int pixel_var = cfg.read<int>("Screen Checks", "pixel-variation", 0);
  if (ImGui::SliderInt(lang.translate("pixel variation").c_str(), &pixel_var, 0, 50)) {
    cfg.write("Screen Checks", "pixel-variation", pixel_var);
  }

  int image_var = cfg.read<int>("Screen Checks", "image-variation", 15);
  if (ImGui::SliderInt(lang.translate("image variation").c_str(), &image_var, 0, 100)) {
    cfg.write("Screen Checks", "image-variation", image_var);
  }
}

void SettingsWindow::render_about() {
  auto& lang = core::Application::instance().language_manager();

  ImGui::SeparatorText(lang.translate("about").c_str());
  ImGui::TextWrapped("Exile UI v1.64.0 (C++ Edition)");
  ImGui::Spacing();
  ImGui::TextWrapped("A versatile overlay tool for Path of Exile and Path of Exile 2.");
  ImGui::TextWrapped("Originally written in AutoHotkey, rewritten in C++20 with Dear ImGui.");
  ImGui::Spacing();
  ImGui::TextWrapped(lang.translate("credits").c_str());
}

} // namespace exile::ui