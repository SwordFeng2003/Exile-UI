cmake_minimum_required(VERSION 3.20)
project(ExileUI VERSION 1.64.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(EXILE_UI_BUILD_TESTS "Build tests" OFF)

find_package(OpenCV QUIET)
find_package(Tesseract QUIET)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.0
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG v0.16
)
FetchContent_MakeAvailable(implot)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
  simpleini
  GIT_REPOSITORY https://github.com/brofield/simpleini.git
  GIT_TAG master
)
FetchContent_MakeAvailable(simpleini)

set(SOURCES
  src/main.cpp
  src/core/application.cpp
  src/core/config_manager.cpp
  src/core/log_manager.cpp
  src/core/language_manager.cpp
  src/core/thread_manager.cpp
  src/core/hotkey_manager.cpp
  src/core/overlay_manager.cpp
  src/core/resource_manager.cpp
  src/game/window_detector.cpp
  src/game/screen_checker.cpp
  src/game/log_parser.cpp
  src/game/clipboard_handler.cpp
  src/game/input_simulator.cpp
  src/game/economy_fetcher.cpp
  src/ui/imgui_renderer.cpp
  src/ui/settings_window.cpp
  src/ui/radial_menu.cpp
  src/ui/widgets.cpp
  src/ui/theme.cpp
  src/modules/clone_frames.cpp
  src/modules/item_checker.cpp
  src/modules/leveling_tracker.cpp
  src/modules/act_decoder.cpp
  src/modules/stash_ninja.cpp
  src/modules/sanctum_planner.cpp
  src/modules/lootfilter.cpp
  src/modules/map_tracker.cpp
  src/modules/exchange.cpp
  src/modules/anoints.cpp
  src/modules/betrayal_info.cpp
  src/modules/cheat_sheets.cpp
  src/modules/macros.cpp
  src/modules/map_info.cpp
  src/modules/ocr_module.cpp
  src/modules/search_strings.cpp
  src/modules/recombination.cpp
  src/modules/qol_tools.cpp
  src/modules/seed_explorer.cpp
  src/modules/statlas.cpp
  src/ui/imgui_impl_dx11.cpp
)

add_executable(exile_ui ${SOURCES})

target_include_directories(exile_ui PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/vendor
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${implot_SOURCE_DIR}
  ${spdlog_SOURCE_DIR}/include
  ${stb_SOURCE_DIR}
  ${simpleini_SOURCE_DIR}
)

target_link_libraries(exile_ui PRIVATE
  imgui
  implot
  spdlog::spdlog
  CURL::libcurl
  nlohmann_json::nlohmann_json
  d3d11
  dxgi
  d3dcompiler
  gdi32
  user32
  shell32
  ole32
)

if(OpenCV_FOUND)
  target_link_libraries(exile_ui PRIVATE ${OpenCV_LIBS})
  target_compile_definitions(exile_ui PRIVATE HAS_OPENCV)
endif()

if(Tesseract_FOUND)
  target_link_libraries(exile_ui PRIVATE ${Tesseract_LIBS})
  target_compile_definitions(exile_ui PRIVATE HAS_TESSERACT)
endif()

target_compile_definitions(exile_ui PRIVATE
  IMGUI_DISABLE_OBSOLETE_FUNCTIONS
  NOMINMAX
  WIN32_LEAN_AND_MEAN
)

if(EXILE_UI_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

install(TARGETS exile_ui RUNTIME DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/exile_ui/data)