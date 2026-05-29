#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace exile::modules {

struct OcrResult {
  std::string text;
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
  float confidence = 0.0f;
};

class OcrModule {
public:
  void initialize();
  void shutdown();

  bool is_ready() const { return initialized_; }

  std::string read_text(int x, int y, int width, int height, int scale = 4);
  std::string read_item_text();
  std::string read_stash_text(int tab_index);
  std::string read_chat_text();

  std::vector<OcrResult> read_text_regions(int x, int y, int width, int height,
                                           int scale = 4);

  void set_language(const std::string& lang) { language_ = lang; }
  void set_whitelist(const std::string& chars) { whitelist_ = chars; }
  void set_blacklist(const std::string& chars) { blacklist_ = chars; }
  void set_scale(int scale) { scale_ = scale; }
  void set_contrast(float contrast) { contrast_ = contrast; }

  std::string sanitize_text(const std::string& text) const;

  void on_mouse_click(int x, int y);
  void on_tick();

private:
  HBITMAP capture_area(int x, int y, int w, int h);
  HBITMAP preprocess_image(HBITMAP source, int& w, int& h);

  bool initialized_ = false;
  std::string language_ = "eng";
  std::string whitelist_;
  std::string blacklist_;
  int scale_ = 4;
  float contrast_ = 1.5f;
  bool scan_pending_ = false;
  int scan_x_ = 0, scan_y_ = 0, scan_w_ = 0, scan_h_ = 0;
};

} // namespace exile::modules