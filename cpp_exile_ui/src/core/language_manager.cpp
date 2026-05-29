#include "core/language_manager.h"
#include <fstream>
#include <algorithm>
#include <cctype>

namespace exile::core {

bool LanguageManager::load_ui_language(const std::filesystem::path& ui_file) {
  ui_translations_ = parse_language_file(ui_file);
  if (ui_lang_ != "english") {
    fallback_translations_ = parse_language_file("data/languages/ui-english.txt");
  }
  return !ui_translations_.empty();
}

bool LanguageManager::load_client_language(const std::filesystem::path& client_file) {
  client_translations_ = parse_language_file(client_file);
  return !client_translations_.empty();
}

std::unordered_map<std::string, std::vector<std::string>> LanguageManager::parse_language_file(
    const std::filesystem::path& file_path) {
  std::unordered_map<std::string, std::vector<std::string>> result;

  if (!std::filesystem::exists(file_path)) return result;

  std::ifstream file(file_path);
  if (!file.is_open()) return result;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;

    auto eq_pos = line.find('=');
    if (eq_pos == std::string::npos) continue;

    auto key = line.substr(0, eq_pos);
    auto value = line.substr(eq_pos + 1);

    for (auto& c : value) {
      if (c == '|') c = '\n';
    }

    result[key].push_back(value);
  }

  return result;
}

std::string LanguageManager::translate(const std::string& key, int index) const {
  auto it = ui_translations_.find(key);
  if (it != ui_translations_.end() && static_cast<size_t>(index - 1) < it->second.size()) {
    return it->second[index - 1];
  }

  if (ui_lang_ != "english") {
    auto fit = fallback_translations_.find(key);
    if (fit != fallback_translations_.end() && static_cast<size_t>(index - 1) < fit->second.size()) {
      return fit->second[index - 1];
    }
  }

  return key;
}

std::string LanguageManager::translate_with_insert(const std::string& key, int index,
                                                     const std::vector<std::string>& inserts) const {
  auto text = translate(key, index);
  for (size_t i = 0; i < inserts.size(); ++i) {
    auto placeholder = "insert" + std::to_string(i + 1);
    auto pos = text.find(placeholder);
    if (pos != std::string::npos) {
      text.replace(pos, placeholder.size(), inserts[i]);
    }
  }
  return text;
}

bool LanguageManager::match(const std::string& text, const std::vector<std::string>& needles,
                            bool case_sensitive) const {
  std::string search_text = text;
  std::string needle_text;
  if (!case_sensitive) {
    std::transform(search_text.begin(), search_text.end(), search_text.begin(),
                   [](unsigned char c) { return std::tolower(c); });
  }

  for (const auto& n : needles) {
    needle_text = n;
    if (!case_sensitive) {
      std::transform(needle_text.begin(), needle_text.end(), needle_text.begin(),
                     [](unsigned char c) { return std::tolower(c); });
    }
    if (search_text.find(needle_text) != std::string::npos) return true;
  }
  return false;
}

std::string LanguageManager::trim(const std::string& text,
                                   const std::vector<std::string>& removals) const {
  std::string result = text;
  for (const auto& r : removals) {
    size_t pos = 0;
    while ((pos = result.find(r, pos)) != std::string::npos) {
      result.erase(pos, r.size());
    }
  }
  return result;
}

std::string LanguageManager::font_name() const {
  if (ui_lang_ == "taiwanese" || ui_lang_ == "korean") return "Karla";
  if (ui_lang_ == "chinese") return "NotoSans";
  return "Fontin";
}

} // namespace exile::core