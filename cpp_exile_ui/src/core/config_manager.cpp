#include "core/config_manager.h"
#include <fstream>
#include <sstream>

namespace exile::core {

bool ConfigManager::load(const std::filesystem::path& ini_path) {
  if (!std::filesystem::exists(ini_path)) return false;

  std::ifstream file(ini_path);
  if (!file.is_open()) return false;

  std::string line;
  std::string current_section;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == ';' || line[0] == '#') continue;

    if (line[0] == '[' && line.back() == ']') {
      current_section = line.substr(1, line.size() - 2);
      continue;
    }

    auto eq_pos = line.find('=');
    if (eq_pos == std::string::npos) continue;

    auto key = line.substr(0, eq_pos);
    auto value = line.substr(eq_pos + 1);

    while (!key.empty() && (key.back() == ' ' || key.back() == '\t' || key.back() == '\r'))
      key.pop_back();
    while (!key.empty() && (key.front() == ' ' || key.front() == '\t'))
      key.erase(0, 1);
    while (!value.empty() && (value.back() == '\r'))
      value.pop_back();

    data_[current_section][key] = value;
  }

  return true;
}

bool ConfigManager::save(const std::filesystem::path& ini_path) const {
  std::ofstream file(ini_path);
  if (!file.is_open()) return false;

  for (const auto& [section, keys] : data_) {
    file << "[" << section << "]\n";
    for (const auto& [key, value] : keys) {
      file << key << "=" << value << "\n";
    }
    file << "\n";
  }

  return true;
}

bool ConfigManager::has_section(const std::string& section) const {
  return data_.find(section) != data_.end();
}

bool ConfigManager::has_key(const std::string& section, const std::string& key) const {
  auto it = data_.find(section);
  if (it == data_.end()) return false;
  return it->second.find(key) != it->second.end();
}

std::vector<std::string> ConfigManager::sections() const {
  std::vector<std::string> result;
  for (const auto& [k, _] : data_) result.push_back(k);
  return result;
}

std::vector<std::string> ConfigManager::keys(const std::string& section) const {
  std::vector<std::string> result;
  auto it = data_.find(section);
  if (it != data_.end()) {
    for (const auto& [k, _] : it->second) result.push_back(k);
  }
  return result;
}

std::unordered_map<std::string, std::string> ConfigManager::section_data(
    const std::string& section) const {
  auto it = data_.find(section);
  if (it != data_.end()) return it->second;
  return {};
}

void ConfigManager::remove_section(const std::string& section) {
  data_.erase(section);
}

void ConfigManager::remove_key(const std::string& section, const std::string& key) {
  auto it = data_.find(section);
  if (it != data_.end()) it->second.erase(key);
}

void ConfigManager::clear() {
  data_.clear();
}

std::string ConfigManager::raw_read(const std::string& section, const std::string& key,
                                     const std::string& default_val) const {
  return read<std::string>(section, key, default_val);
}

} // namespace exile::core