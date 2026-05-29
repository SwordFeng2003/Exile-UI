#include "core/application.h"
#include "core/log_manager.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/thread_manager.h"
#include "core/hotkey_manager.h"
#include "core/overlay_manager.h"
#include "core/resource_manager.h"
#include "game/window_detector.h"
#include "game/screen_checker.h"
#include "game/log_parser.h"
#include "game/clipboard_handler.h"
#include "game/input_simulator.h"
#include "ui/imgui_renderer.h"
#include "ui/settings_window.h"
#include "ui/radial_menu.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include "modules/clone_frames.h"
#include "modules/item_checker.h"
#include "modules/leveling_tracker.h"
#include "modules/act_decoder.h"
#include "modules/stash_ninja.h"
#include "modules/sanctum_planner.h"
#include "modules/lootfilter.h"
#include "modules/map_tracker.h"
#include "modules/exchange.h"
#include "modules/anoints.h"
#include "modules/betrayal_info.h"
#include "modules/cheat_sheets.h"
#include "modules/macros.h"
#include "modules/map_info.h"
#include "modules/ocr_module.h"
#include "modules/search_strings.h"
#include "modules/recombination.h"
#include "modules/qol_tools.h"
#include "modules/seed_explorer.h"
#include "modules/statlas.h"

#include <memory>

using namespace exile;

static std::unique_ptr<game::WindowDetector> g_window_detector;
static std::unique_ptr<game::ScreenChecker> g_screen_checker;
static std::unique_ptr<game::LogParser> g_log_parser;
static std::unique_ptr<game::EconomyFetcher> g_economy_fetcher;

static std::unique_ptr<ui::ImGuiRenderer> g_imgui_renderer;
static std::unique_ptr<ui::SettingsWindow> g_settings_window;
static std::unique_ptr<ui::RadialMenu> g_radial_menu;

static std::unique_ptr<modules::CloneFrames> g_clone_frames;
static std::unique_ptr<modules::ItemChecker> g_item_checker;
static std::unique_ptr<modules::LevelingTracker> g_leveling_tracker;
static std::unique_ptr<modules::ActDecoder> g_act_decoder;
static std::unique_ptr<modules::StashNinja> g_stash_ninja;
static std::unique_ptr<modules::SanctumPlanner> g_sanctum_planner;
static std::unique_ptr<modules::Lootfilter> g_lootfilter;
static std::unique_ptr<modules::MapTracker> g_map_tracker;
static std::unique_ptr<modules::Exchange> g_exchange;
static std::unique_ptr<modules::Anoints> g_anoints;
static std::unique_ptr<modules::BetrayalInfo> g_betrayal_info;
static std::unique_ptr<modules::CheatSheets> g_cheat_sheets;
static std::unique_ptr<modules::Macros> g_macros;
static std::unique_ptr<modules::MapInfo> g_map_info;
static std::unique_ptr<modules::OcrModule> g_ocr_module;
static std::unique_ptr<modules::SearchStrings> g_search_strings;
static std::unique_ptr<modules::Recombination> g_recombination;
static std::unique_ptr<modules::QolTools> g_qol_tools;
static std::unique_ptr<modules::SeedExplorer> g_seed_explorer;
static std::unique_ptr<modules::Statlas> g_statlas;

static void initialize_all_modules() {
  auto& app = core::Application::instance();
  auto& log = core::LogManager::instance();

  log.info("Initializing core modules...");

  g_clone_frames = std::make_unique<modules::CloneFrames>();
  g_item_checker = std::make_unique<modules::ItemChecker>();
  g_leveling_tracker = std::make_unique<modules::LevelingTracker>();
  g_act_decoder = std::make_unique<modules::ActDecoder>();
  g_stash_ninja = std::make_unique<modules::StashNinja>();
  g_sanctum_planner = std::make_unique<modules::SanctumPlanner>();
  g_lootfilter = std::make_unique<modules::Lootfilter>();
  g_map_tracker = std::make_unique<modules::MapTracker>();
  g_exchange = std::make_unique<modules::Exchange>();
  g_anoints = std::make_unique<modules::Anoints>();
  g_betrayal_info = std::make_unique<modules::BetrayalInfo>();
  g_cheat_sheets = std::make_unique<modules::CheatSheets>();
  g_macros = std::make_unique<modules::Macros>();
  g_map_info = std::make_unique<modules::MapInfo>();
  g_ocr_module = std::make_unique<modules::OcrModule>();
  g_search_strings = std::make_unique<modules::SearchStrings>();
  g_recombination = std::make_unique<modules::Recombination>();
  g_qol_tools = std::make_unique<modules::QolTools>();
  g_seed_explorer = std::make_unique<modules::SeedExplorer>();
  g_statlas = std::make_unique<modules::Statlas>();

  g_clone_frames->initialize();
  g_item_checker->initialize();
  g_leveling_tracker->initialize();
  g_act_decoder->initialize();
  g_stash_ninja->initialize();
  g_sanctum_planner->initialize();
  g_lootfilter->initialize();
  g_map_tracker->initialize();
  g_exchange->initialize();
  g_anoints->initialize();
  g_betrayal_info->initialize();
  g_cheat_sheets->initialize();
  g_macros->initialize();
  g_map_info->initialize();
  g_ocr_module->initialize();
  g_search_strings->initialize();
  g_recombination->initialize();
  g_qol_tools->initialize();
  g_seed_explorer->initialize();
  g_statlas->initialize();

  log.info("All modules initialized successfully");
}

static void shutdown_all_modules() {
  g_clone_frames->shutdown();
  g_item_checker->shutdown();
  g_leveling_tracker->shutdown();
  g_act_decoder->shutdown();
  g_stash_ninja->shutdown();
  g_sanctum_planner->shutdown();
  g_lootfilter->shutdown();
  g_map_tracker->shutdown();
  g_exchange->shutdown();
  g_anoints->shutdown();
  g_betrayal_info->shutdown();
  g_cheat_sheets->shutdown();
  g_macros->shutdown();
  g_map_info->shutdown();
  g_ocr_module->shutdown();
  g_search_strings->shutdown();
  g_recombination->shutdown();
  g_qol_tools->shutdown();
  g_seed_explorer->shutdown();
  g_statlas->shutdown();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  auto& app = core::Application::instance();
  auto& log = core::LogManager::instance();

  core::ApplicationConfig config;
  config.dev_mode = false;

  log.initialize("logs/exile_ui.log", true);
  log.info("Exile UI starting...");

  if (!app.initialize(config)) {
    log.error("Failed to initialize application");
    return 1;
  }

  log.info("Detecting game window...");
  g_window_detector = std::make_unique<game::WindowDetector>();
  if (!g_window_detector->detect_game_window()) {
    log.warn("Game window not found, continuing in background...");
  }

  g_screen_checker = std::make_unique<game::ScreenChecker>();
  g_log_parser = std::make_unique<game::LogParser>();
  g_economy_fetcher = std::make_unique<game::EconomyFetcher>();

  auto& client = g_window_detector->client();
  g_screen_checker->initialize(
    client.poe_version, client.height,
    client.x, client.y, client.w, client.h, 0);

  auto log_path = g_window_detector->find_log_file_path();
  if (!log_path.empty()) {
    g_log_parser->initialize(log_path);
  }

  initialize_all_modules();

  app.run();

  shutdown_all_modules();

  g_log_parser->shutdown();
  app.shutdown();

  return 0;
}