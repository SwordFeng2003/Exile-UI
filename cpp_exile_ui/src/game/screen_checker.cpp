#include "game/screen_checker.h"
#include "core/resource_manager.h"
#include <algorithm>

namespace exile::game {

static uint32_t get_pixel_color(HDC dc, int x, int y) {
  COLORREF color = GetPixel(dc, x, y);
  return ((GetRValue(color) & 0xFF) << 16) |
         ((GetGValue(color) & 0xFF) << 8) |
         (GetBValue(color) & 0xFF);
}

static bool color_match(uint32_t c1, uint32_t c2, int variation) {
  int dr = abs(static_cast<int>((c1 >> 16) & 0xFF) - static_cast<int>((c2 >> 16) & 0xFF));
  int dg = abs(static_cast<int>((c1 >> 8) & 0xFF) - static_cast<int>((c2 >> 8) & 0xFF));
  int db = abs(static_cast<int>(c1 & 0xFF) - static_cast<int>(c2 & 0xFF));
  return dr <= variation && dg <= variation && db <= variation;
}

void ScreenChecker::initialize(const std::string& poe_version, int client_height,
                                int client_x, int client_y, int client_w, int client_h,
                                int black_bar_offset) {
  poe_version_ = poe_version;
  client_x_ = client_x;
  client_y_ = client_y;
  client_w_ = client_w;
  client_h_ = client_h;
  black_bar_offset_ = black_bar_offset;
}

bool ScreenChecker::check_gamescreen() {
  if (image_check_order_.empty()) return false;

  for (auto& name : image_check_order_) {
    auto it = image_checks_.find(name);
    if (it == image_checks_.end()) continue;

    auto& check = it->second;
    HDC dc = GetDC(nullptr);
    HDC mem_dc = CreateCompatibleDC(dc);
    HDC temp_dc = CreateCompatibleDC(dc);

    HBITMAP screen = CreateCompatibleBitmap(dc, client_w_, client_h_);
    SelectObject(mem_dc, screen);
    BitBlt(mem_dc, 0, 0, client_w_, client_h_, dc, client_x_, client_y_, SRCCOPY);

    HBITMAP source = core::Application::instance().resource_manager().get_image(name);
    if (source) {
      BITMAP bm;
      GetObject(source, sizeof(BITMAP), &bm);

      SelectObject(temp_dc, source);

      bool found = false;
      int sx = 0, sy = 0;

      if (TransparentBlt(mem_dc, check.x1, check.y1, check.x2 - check.x1, check.y2 - check.y1,
                          temp_dc, 0, 0, bm.bmWidth, bm.bmHeight, RGB(0, 0, 0))) {
        found = true;
        sx = check.x1;
        sy = check.y1;
      }

      check.last_result = found;
      if (found) {
        check.found_x = sx;
        check.found_y = sy;
        check.found_w = bm.bmWidth;
        check.found_h = bm.bmHeight;
      }
    }

    DeleteDC(temp_dc);
    DeleteDC(mem_dc);
    DeleteObject(screen);
    ReleaseDC(nullptr, dc);
  }

  return true;
}

bool ScreenChecker::check_inventory() {
  auto& cfg = core::Application::instance().config_manager();
  bool in_inventory = false;

  int ix1 = cfg.read<int>("Screen Checks", "StashLoc1_x", 0);
  int iy1 = cfg.read<int>("Screen Checks", "StashLoc1_y", 0);
  int ix2 = cfg.read<int>("Screen Checks", "StashLoc2_x", client_w_);
  int iy2 = cfg.read<int>("Screen Checks", "StashLoc2_y", client_h_);

  HDC dc = GetDC(nullptr);
  auto pixel = get_pixel_color(dc, ix1, iy1);
  ReleaseDC(nullptr, dc);

  uint32_t expected = static_cast<uint32_t>(
    cfg.read<int>("Screen Checks", "StashLoc1_col", 0));
  if (color_match(pixel, expected, pixel_variation_)) {
    in_inventory = true;
  }

  return in_inventory;
}

bool ScreenChecker::check_close_button() {
  auto& cfg = core::Application::instance().config_manager();

  int bx = cfg.read<int>("Screen Checks", "closeButton_x1", 0);
  int by = cfg.read<int>("Screen Checks", "closeButton_y1", 0);
  if (bx == 0 && by == 0) return false;

  HDC dc = GetDC(nullptr);
  auto pixel = get_pixel_color(dc, bx, by);
  ReleaseDC(nullptr, dc);

  uint32_t expected = static_cast<uint32_t>(
    cfg.read<int>("Screen Checks", "closeButton_col", 0));
  return color_match(pixel, expected, pixel_variation_);
}

bool ScreenChecker::perform_pixel_search(const std::string& name) {
  auto it = pixel_checks_.find(name);
  if (it == pixel_checks_.end()) return false;

  auto& check = it->second;
  HDC dc = GetDC(nullptr);

  auto p1 = get_pixel_color(dc, check.x1, check.y1);
  auto p2 = get_pixel_color(dc, check.x2, check.y2);
  auto p3 = get_pixel_color(dc, check.x3, check.y3);

  ReleaseDC(nullptr, dc);

  return color_match(p1, check.color1, pixel_variation_) &&
         color_match(p2, check.color2, pixel_variation_) &&
         color_match(p3, check.color3, pixel_variation_);
}

bool ScreenChecker::check_skilltree() {
  return perform_pixel_search("skilltree");
}

bool ScreenChecker::check_atlas() {
  return perform_pixel_search("atlas");
}

bool ScreenChecker::check_betrayal() {
  auto& cfg = core::Application::instance().config_manager();
  std::string check_name = cfg.read<std::string>("Screen Checks", "betrayal-check", "betrayal");
  return perform_pixel_search(check_name);
}

bool ScreenChecker::check_sanctum() {
  return perform_pixel_search("sanctum");
}

bool ScreenChecker::check_exchange() {
  return perform_pixel_search("exchange");
}

bool ScreenChecker::check_stash() {
  auto& cfg = core::Application::instance().config_manager();
  std::string check_name = cfg.read<std::string>("Screen Checks", "stash-check", "stash");
  return perform_pixel_search(check_name);
}

bool ScreenChecker::check_async_trade(int type) {
  if (type == 2) return perform_pixel_search("exchange");
  return perform_pixel_search("trade");
}

void ScreenChecker::perform_all_image_checks() {
  for (auto& name : image_check_order_) {
    auto it = image_checks_.find(name);
    if (it == image_checks_.end()) continue;

    check_gamescreen();
  }
}

void ScreenChecker::perform_all_pixel_checks() {
  for (auto& name : pixel_check_order_) {
    perform_pixel_search(name);
  }
}

bool ScreenChecker::recalibrate_pixel(const std::string& name) {
  auto it = pixel_checks_.find(name);
  if (it == pixel_checks_.end()) return false;

  auto& check = it->second;
  HDC dc = GetDC(nullptr);

  check.color1 = get_pixel_color(dc, check.x1, check.y1);
  check.color2 = get_pixel_color(dc, check.x2, check.y2);
  check.color3 = get_pixel_color(dc, check.x3, check.y3);

  ReleaseDC(nullptr, dc);

  auto& cfg = core::Application::instance().config_manager();
  cfg.write("Screen Checks", name + "_col1", static_cast<int>(check.color1));

  return true;
}

bool ScreenChecker::recalibrate_image(const std::string& name) {
  auto it = image_checks_.find(name);
  if (it == image_checks_.end()) return false;
  return true;
}

const ImageCheck& ScreenChecker::image_check(const std::string& name) const {
  static ImageCheck empty;
  auto it = image_checks_.find(name);
  if (it != image_checks_.end()) return it->second;
  return empty;
}

const PixelCheck& ScreenChecker::pixel_check(const std::string& name) const {
  static PixelCheck empty;
  auto it = pixel_checks_.find(name);
  if (it != pixel_checks_.end()) return it->second;
  return empty;
}

std::vector<std::string> ScreenChecker::enabled_image_checks() const {
  return image_check_order_;
}

std::vector<std::string> ScreenChecker::enabled_pixel_checks() const {
  return pixel_check_order_;
}

HBITMAP ScreenChecker::capture_screen_area(int x, int y, int w, int h) {
  HDC dc = GetDC(nullptr);
  HDC mem_dc = CreateCompatibleDC(dc);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, w, h);
  SelectObject(mem_dc, bitmap);
  BitBlt(mem_dc, 0, 0, w, h, dc, x, y, SRCCOPY);
  DeleteDC(mem_dc);
  ReleaseDC(nullptr, dc);
  return bitmap;
}

HBITMAP ScreenChecker::capture_client_area(int x, int y, int w, int h) {
  return capture_screen_area(client_x_ + x + black_bar_offset_, client_y_ + y, w, h);
}

} // namespace exile::game