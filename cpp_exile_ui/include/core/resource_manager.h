#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <Windows.h>

namespace exile::core {

class ResourceManager {
public:
  bool load_image(const std::string& name, const std::filesystem::path& path);
  bool load_image_resized(const std::string& name, const std::filesystem::path& path,
                          int width, int height);
  HBITMAP get_image(const std::string& name) const;
  bool has_image(const std::string& name) const;
  void unload_image(const std::string& name);
  void unload_all();

  bool load_json(const std::string& name, const std::filesystem::path& path);
  bool has_json(const std::string& name) const;
  std::string get_json_string(const std::string& name) const;

  bool load_text_file(const std::string& name, const std::filesystem::path& path,
                      bool lowercase = false);

  std::string resolve_path(const std::string& relative_path) const;
  void set_base_path(const std::filesystem::path& base) { base_path_ = base; }

private:
  std::filesystem::path base_path_;
  std::unordered_map<std::string, HBITMAP> images_;
  std::unordered_map<std::string, std::string> json_data_;
  std::unordered_map<std::string, std::string> text_data_;
};

} // namespace exile::core