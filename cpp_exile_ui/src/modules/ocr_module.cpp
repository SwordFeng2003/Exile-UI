#include "modules/ocr_module.h"
#include "core/config_manager.h"
#include "core/application.h"
#include "core/log_manager.h"
#include <algorithm>

namespace exile::modules {

void OcrModule::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  language_ = cfg.read<std::string>("OCR", "language", "eng");
  scale_ = cfg.read<int>("OCR", "scale", 4);
  contrast_ = cfg.read<float>("OCR", "contrast", 1.5f);
  whitelist_ = cfg.read<std::string>("OCR", "whitelist", "");
  blacklist_ = cfg.read<std::string>("OCR", "blacklist", "");

  initialized_ = true;
}

void OcrModule::shutdown() {
  initialized_ = false;
}

std::string OcrModule::read_text(int x, int y, int width, int height, int scale) {
  if (!initialized_) return "";

  HBITMAP capture = capture_area(x, y, width, height);
  if (!capture) return "";

  DeleteObject(capture);

#ifdef HAS_TESSERACT
  int pw = 0, ph = 0;
  HBITMAP processed = preprocess_image(capture, pw, ph);
  if (processed) DeleteObject(processed);
#endif

  return "";
}

std::string OcrModule::read_item_text() {
  return read_text(0, 0, 400, 300, scale_);
}

std::string OcrModule::read_stash_text(int tab_index) {
  int start_x = 492;
  int width = 350;
  int height = 200;
  return read_text(start_x, 0, width, height, scale_);
}

std::string OcrModule::read_chat_text() {
  int x = 8;
  int y = 200;
  int width = 500;
  int height = 400;
  return read_text(x, y, width, height, scale_);
}

std::vector<OcrResult> OcrModule::read_text_regions(int x, int y, int width, int height, int scale) {
  std::vector<OcrResult> results;
#ifdef HAS_TESSERACT
#endif
  return results;
}

std::string OcrModule::sanitize_text(const std::string& text) const {
  std::string result;
  for (char c : text) {
    if (c >= 0x20 && c <= 0x7E) result += c;
    if (c == '\n') result += ' ';
  }

  std::string trimmed;
  bool last_was_space = false;
  for (char c : result) {
    if (c == ' ') {
      if (!last_was_space) {
        trimmed += c;
        last_was_space = true;
      }
    } else {
      trimmed += c;
      last_was_space = false;
    }
  }

  return trimmed;
}

void OcrModule::on_mouse_click(int x, int y) {
}

void OcrModule::on_tick() {
  if (scan_pending_) {
    read_text(scan_x_, scan_y_, scan_w_, scan_h_, scale_);
    scan_pending_ = false;
  }
}

HBITMAP OcrModule::capture_area(int x, int y, int w, int h) {
  HDC dc = GetDC(nullptr);
  HDC mem_dc = CreateCompatibleDC(dc);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, w, h);
  SelectObject(mem_dc, bitmap);
  BitBlt(mem_dc, 0, 0, w, h, dc, x, y, SRCCOPY);
  DeleteDC(mem_dc);
  ReleaseDC(nullptr, dc);
  return bitmap;
}

HBITMAP OcrModule::preprocess_image(HBITMAP source, int& w, int& h) {
  BITMAP bm;
  GetObject(source, sizeof(BITMAP), &bm);
  w = bm.bmWidth;
  h = bm.bmHeight;

  HDC src_dc = CreateCompatibleDC(nullptr);
  SelectObject(src_dc, source);

  HDC dst_dc = CreateCompatibleDC(nullptr);
  HBITMAP result = CreateCompatibleBitmap(src_dc, w, h);
  SelectObject(dst_dc, result);

  BitBlt(dst_dc, 0, 0, w, h, src_dc, 0, 0, SRCCOPY);

  DeleteDC(src_dc);
  DeleteDC(dst_dc);

  return result;
}

} // namespace exile::modules