#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace exile::core {

class LanguageManager {
public:
  struct TranslationKey {
    std::string key;
    int index = 1;
  };

  bool load_ui_language(const std::filesystem::path& ui_file);
  bool load_client_language(const std::filesystem::path& client_file);

  std::string translate(const std::string& key, int index = 1) const;
  std::string translate_with_insert(const std::string& key, int index,
                                   const std::vector<std::string>& inserts) const;

  bool match(const std::string& text, const std::vector<std::string>& needles, bool case_sensitive = true) const;
  std::string trim(const std::string& text, const std::vector<std::string>& removals) const;

  void set_ui_lang(const std::string& lang) { ui_lang_ = lang; }
  void set_client_lang(const std::string& lang) { client_lang_ = lang; }
  const std::string& ui_lang() const { return ui_lang_; }
  const std::string& client_lang() const { return client_lang_; }

  const std::unordered_map<std::string, std::vector<std::string>>& ui_translations() const {
    return ui_translations_;
  }
  const std::unordered_map<std::string, std::vector<std::string>>& client_translations() const {
    return client_translations_;
  }

  std::string font_name() const;

private:
  std::unordered_map<std::string, std::vector<std::string>> parse_language_file(
    const std::filesystem::path& file_path);

  std::string ui_lang_ = "english";
  std::string client_lang_ = "english";
  std::unordered_map<std::string, std::vector<std::string>> ui_translations_;
  std::unordered_map<std::string, std::vector<std::string>> client_translations_;
  std::unordered_map<std::string, std::vector<std::string>> fallback_translations_;
};

} // namespace exile::core