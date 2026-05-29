#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <filesystem>

namespace exile::core {

class ConfigManager {
public:
  bool load(const std::filesystem::path& ini_path);
  bool save(const std::filesystem::path& ini_path) const;

  template<typename T>
  T read(const std::string& section, const std::string& key, T default_val = T{}) const {
    auto sit = data_.find(section);
    if (sit == data_.end()) return default_val;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return default_val;
    try {
      if constexpr (std::is_same_v<T, int>) return std::stoi(kit->second);
      else if constexpr (std::is_same_v<T, float>) return std::stof(kit->second);
      else if constexpr (std::is_same_v<T, double>) return std::stod(kit->second);
      else if constexpr (std::is_same_v<T, bool>) return kit->second == "1" || kit->second == "true";
      else return T{kit->second};
    } catch (...) { return default_val; }
  }

  template<typename T>
  void write(const std::string& section, const std::string& key, T value) {
    if constexpr (std::is_same_v<T, bool>)
      data_[section][key] = value ? "1" : "0";
    else if constexpr (std::is_same_v<T, std::string>)
      data_[section][key] = value;
    else
      data_[section][key] = std::to_string(value);
  }

  bool has_section(const std::string& section) const;
  bool has_key(const std::string& section, const std::string& key) const;
  std::vector<std::string> sections() const;
  std::vector<std::string> keys(const std::string& section) const;
  std::unordered_map<std::string, std::string> section_data(const std::string& section) const;

  void remove_section(const std::string& section);
  void remove_key(const std::string& section, const std::string& key);
  void clear();

  std::string raw_read(const std::string& section, const std::string& key,
                       const std::string& default_val = "") const;

private:
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data_;
};

} // namespace exile::core