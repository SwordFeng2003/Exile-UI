#include "core/resource_manager.h"
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace exile::core {

static HBITMAP create_hbitmap_from_rgba(const uint8_t* data, int w, int h) {
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = w;
  bmi.bmiHeader.biHeight = -h;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* bits = nullptr;
  HDC dc = GetDC(nullptr);
  HBITMAP bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
  ReleaseDC(nullptr, dc);

  if (bits) {
    memcpy(bits, data, w * h * 4);
  }

  return bmp;
}

bool ResourceManager::load_image(const std::string& name, const std::filesystem::path& path) {
  if (images_.find(name) != images_.end()) return true;

  int w = 0, h = 0, channels = 0;
  auto* data = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
  if (!data) return false;

  HBITMAP bmp = create_hbitmap_from_rgba(data, w, h);
  stbi_image_free(data);

  if (!bmp) return false;

  images_[name] = bmp;
  return true;
}

bool ResourceManager::load_image_resized(const std::string& name,
                                          const std::filesystem::path& path,
                                          int width, int height) {
  int w = 0, h = 0, channels = 0;
  auto* data = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
  if (!data) return false;

  std::vector<uint8_t> resized(width * height * 4);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int sx = x * w / width;
      int sy = y * h / height;
      for (int c = 0; c < 4; ++c) {
        resized[(y * width + x) * 4 + c] = data[(sy * w + sx) * 4 + c];
      }
    }
  }

  stbi_image_free(data);

  HBITMAP bmp = create_hbitmap_from_rgba(resized.data(), width, height);
  if (!bmp) return false;

  images_[name] = bmp;
  return true;
}

HBITMAP ResourceManager::get_image(const std::string& name) const {
  auto it = images_.find(name);
  if (it != images_.end()) return it->second;
  return nullptr;
}

bool ResourceManager::has_image(const std::string& name) const {
  return images_.find(name) != images_.end();
}

void ResourceManager::unload_image(const std::string& name) {
  auto it = images_.find(name);
  if (it != images_.end()) {
    DeleteObject(it->second);
    images_.erase(it);
  }
}

void ResourceManager::unload_all() {
  for (auto& [_, bmp] : images_) {
    DeleteObject(bmp);
  }
  images_.clear();
}

bool ResourceManager::load_json(const std::string& name, const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file.is_open()) return false;

  std::stringstream buffer;
  buffer << file.rdbuf();
  json_data_[name] = buffer.str();
  return true;
}

bool ResourceManager::has_json(const std::string& name) const {
  return json_data_.find(name) != json_data_.end();
}

std::string ResourceManager::get_json_string(const std::string& name) const {
  auto it = json_data_.find(name);
  if (it != json_data_.end()) return it->second;
  return "";
}

bool ResourceManager::load_text_file(const std::string& name,
                                      const std::filesystem::path& path,
                                      bool lowercase) {
  std::ifstream file(path);
  if (!file.is_open()) return false;

  std::stringstream buffer;
  std::string line;
  while (std::getline(file, line)) {
    if (lowercase) {
      for (auto& c : line) c = std::tolower(c);
    }
    buffer << line << "\n";
  }
  text_data_[name] = buffer.str();
  return true;
}

std::string ResourceManager::resolve_path(const std::string& relative_path) const {
  return (base_path_ / relative_path).string();
}

} // namespace exile::core