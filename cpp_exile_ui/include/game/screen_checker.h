#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <Windows.h>

namespace exile::game {

struct PixelCheck {
  int x1 = 0, y1 = 0;
  int x2 = 0, y2 = 0;
  int x3 = 0, y3 = 0;
  uint32_t color1 = 0;
  uint32_t color2 = 0;
  uint32_t color3 = 0;
};

struct ImageCheck {
  int x1 = 0, y1 = 0;
  int x2 = 0, y2 = 0;
  int width = 0, height = 0;
  bool last_result = false;
  int found_x = 0, found_y = 0;
  int found_w = 0, found_h = 0;
};

class ScreenChecker {
public:
  void initialize(const std::string& poe_version, int client_height,
                  int client_x, int client_y, int client_w, int client_h,
                  int black_bar_offset = 0);

  bool check_gamescreen();
  bool check_inventory();
  bool check_close_button();
  bool perform_pixel_search(const std::string& name);

  bool check_skilltree();
  bool check_atlas();
  bool check_betrayal();
  bool check_sanctum();
  bool check_exchange();
  bool check_stash();
  bool check_async_trade(int type = 1);

  void perform_all_image_checks();
  void perform_all_pixel_checks();

  bool recalibrate_pixel(const std::string& name);
  bool recalibrate_image(const std::string& name);

  void set_variation(int pixel_var, int image_var) {
    pixel_variation_ = pixel_var;
    image_variation_ = image_var;
  }

  const ImageCheck& image_check(const std::string& name) const;
  const PixelCheck& pixel_check(const std::string& name) const;

  std::vector<std::string> enabled_image_checks() const;
  std::vector<std::string> enabled_pixel_checks() const;

  HBITMAP capture_screen_area(int x, int y, int w, int h);
  HBITMAP capture_client_area(int x, int y, int w, int h);

private:
  bool image_search(const std::string& name, HBITMAP haystack, int x1, int y1, int x2, int y2);

  int client_x_ = 0, client_y_ = 0;
  int client_w_ = 1920, client_h_ = 1080;
  int black_bar_offset_ = 0;
  int pixel_variation_ = 0;
  int image_variation_ = 15;
  std::string poe_version_;

  std::unordered_map<std::string, PixelCheck> pixel_checks_;
  std::unordered_map<std::string, ImageCheck> image_checks_;
  std::vector<std::string> image_check_order_;
  std::vector<std::string> pixel_check_order_;
};

} // namespace exile::game