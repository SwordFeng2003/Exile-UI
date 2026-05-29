#include "game/log_parser.h"
#include "core/log_manager.h"
#include <regex>

namespace exile::game {

bool LogParser::initialize(const std::filesystem::path& log_path) {
  log_path_ = log_path;

  if (std::filesystem::exists(log_path)) {
    log_stream_.open(log_path);
    if (log_stream_.is_open()) {
      log_stream_.seekg(0, std::ios::end);
      last_position_ = log_stream_.tellg();
      last_file_size_ = std::filesystem::file_size(log_path);
    }
  }

  return true;
}

void LogParser::shutdown() {
  if (log_stream_.is_open()) {
    log_stream_.close();
  }
}

void LogParser::parse_loop() {
  if (!std::filesystem::exists(log_path_)) return;

  auto current_size = std::filesystem::file_size(log_path_);

  if (current_size < last_file_size_) {
    log_stream_.close();
    log_stream_.open(log_path_);
    last_position_ = 0;
  }
  last_file_size_ = current_size;

  if (!log_stream_.is_open()) {
    log_stream_.open(log_path_);
    if (log_stream_.is_open()) {
      log_stream_.seekg(0, std::ios::end);
    }
    return;
  }

  log_stream_.clear();
  log_stream_.seekg(last_position_);

  std::string line;
  std::vector<LogEntry> new_entries;

  while (std::getline(log_stream_, line)) {
    LogEntry entry;
    parse_line(line, entry);

    if (entry.raw_line.empty()) {
      entry.raw_line = line;
    }

    new_entries.push_back(entry);
  }

  last_position_ = log_stream_.tellg();

  if (!new_entries.empty()) {
    std::lock_guard<std::mutex> lock(entries_mutex_);
    for (auto& entry : new_entries) {
      if (!entry.area_id.empty()) {
        current_area_id_ = entry.area_id;
        current_area_name_ = entry.area_name;
        current_area_level_ = entry.area_level;
        current_area_seed_ = entry.area_seed;
        current_area_tier_ = entry.area_tier;
        current_act_ = entry.act;
        current_player_level_ = entry.player_level;
      }
      pending_entries_.push_back(std::move(entry));
    }
  }
}

bool LogParser::has_new_data() {
  std::lock_guard<std::mutex> lock(entries_mutex_);
  return !pending_entries_.empty();
}

std::vector<LogEntry> LogParser::get_new_entries() {
  std::vector<LogEntry> result;
  std::lock_guard<std::mutex> lock(entries_mutex_);
  result.swap(pending_entries_);
  return result;
}

void LogParser::parse_line(const std::string& line, LogEntry& entry) {
  if (line.empty()) return;

  if (line.find("Generating level") != std::string::npos) {
    handle_generating_level(line, entry);
    return;
  }

  if (line.find("[INFO Client") != std::string::npos) {
    entry.raw_line = line;

    auto bracket_idx = line.find(']');
    if (bracket_idx != std::string::npos) {
      entry.timestamp = line.substr(0, bracket_idx + 1);
    }

    extract_player_info(line);
  }

  if (line.find("has been slain") != std::string::npos) {
    handle_kill(line);
  }

  if (line.find("has been killed") != std::string::npos ||
      line.find("died!") != std::string::npos) {
    handle_death(line);
  }
}

void LogParser::extract_player_info(const std::string& line) {
  std::regex level_regex(R"(:\s(\w+)\s\((\w+)\)\sis\snow\slevel\s(\d+))");
  std::smatch match;
  if (std::regex_search(line, match, level_regex) && match.size() >= 4) {
    player_info_.character_name = match[1];
    player_info_.character_class = match[2];
    player_info_.level = std::stoi(match[3]);
  }
}

void LogParser::handle_generating_level(const std::string& line, LogEntry& entry) {
  entry.raw_line = line;

  auto bracket_idx = line.find(']');
  if (bracket_idx != std::string::npos) {
    entry.timestamp = line.substr(0, bracket_idx);
  }

  std::regex area_regex(R"(Generating\slevel\s(\d+)\sarea\s"([^"]+)"\swith\sseed\s(\d+))");
  std::smatch match;
  if (std::regex_search(line, match, area_regex) && match.size() >= 4) {
    entry.area_level = std::stoi(match[1]);
    entry.area_name = match[2];
    entry.area_seed = std::stoi(match[3]);
  } else {
    std::regex simple_regex(R"(Generating\slevel\s(\d+)\sarea\s"([^"]+)")");
    std::smatch sm;
    if (std::regex_search(line, sm, simple_regex) && sm.size() >= 3) {
      entry.area_level = std::stoi(sm[1]);
      entry.area_name = sm[2];
    }
  }
}

void LogParser::handle_kill(const std::string& line) {
  kill_count_++;
}

void LogParser::handle_death(const std::string& line) {
  death_count_++;
}

bool LogParser::is_in_town() const {
  return current_area_name_.find("Town") != std::string::npos ||
         current_area_name_.find("Hideout") != std::string::npos ||
         current_area_name_.find("Menagerie") != std::string::npos;
}

bool LogParser::is_in_hideout() const {
  return current_area_name_.find("Hideout") != std::string::npos;
}

bool LogParser::is_in_map() const {
  return current_area_tier_ > 0 && !is_in_town();
}

bool LogParser::is_in_labyrinth() const {
  return current_area_name_.find("Labyrinth") != std::string::npos;
}

bool LogParser::is_in_sanctum() const {
  return current_area_name_.find("Sanctum") != std::string::npos;
}

bool LogParser::backup_log() {
  if (!std::filesystem::exists(log_path_)) return false;

  auto now = std::time(nullptr);
  auto tm = *std::localtime(&now);
  char buf[64];
  std::strftime(buf, sizeof(buf), "_%Y%m%d_%H%M%S", &tm);

  auto backup_path = log_path_.string() + buf + ".bak";
  return std::filesystem::copy_file(log_path_, backup_path,
                                     std::filesystem::copy_options::overwrite_existing);
}

LogParser::MapInfo LogParser::get_map_info() const {
  MapInfo info;
  info.name = current_area_name_;
  info.tier = current_area_tier_;
  return info;
}

} // namespace exile::game