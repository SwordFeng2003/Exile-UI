#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>

namespace exile::game {

struct LogEntry {
  std::string raw_line;
  std::string timestamp;
  std::string area_id;
  std::string area_name;
  int area_seed = 0;
  int area_level = 0;
  int area_tier = 0;
  int act = 0;
  int player_level = 0;
  std::string player_class;
  std::string character_name;
};

struct PlayerInfo {
  int level = 0;
  std::string character_class;
  std::string character_name;
  std::string last_area;
};

class LogParser {
public:
  bool initialize(const std::filesystem::path& log_path);
  void shutdown();

  void parse_loop();
  bool has_new_data();
  std::vector<LogEntry> get_new_entries();

  PlayerInfo get_player_info() const { return player_info_; }
  void set_active_character(const std::string& name) { active_character_ = name; }
  const std::string& active_character() const { return active_character_; }

  const std::string& current_area_id() const { return current_area_id_; }
  const std::string& current_area_name() const { return current_area_name_; }
  int current_area_seed() const { return current_area_seed_; }
  int current_area_level() const { return current_area_level_; }
  int current_area_tier() const { return current_area_tier_; }
  int current_act() const { return current_act_; }
  int current_player_level() const { return current_player_level_; }

  bool is_in_town() const;
  bool is_in_hideout() const;
  bool is_in_map() const;
  bool is_in_labyrinth() const;
  bool is_in_sanctum() const;

  bool backup_log();

  struct MapInfo {
    std::string name;
    std::string english_name;
    int tier = 0;
    bool is_unique = false;
  };
  MapInfo get_map_info() const;

private:
  void parse_line(const std::string& line, LogEntry& entry);
  void extract_player_info(const std::string& line);
  void handle_generating_level(const std::string& line, LogEntry& entry);
  void handle_kill(const std::string& line);
  void handle_death(const std::string& line);

  std::filesystem::path log_path_;
  std::ifstream log_stream_;
  uint64_t last_position_ = 0;
  uint64_t last_file_size_ = 0;

  PlayerInfo player_info_;
  std::string active_character_;
  std::string current_area_id_;
  std::string current_area_name_;
  int current_area_seed_ = 0;
  int current_area_level_ = 0;
  int current_area_tier_ = 0;
  int current_act_ = 0;
  int current_player_level_ = 0;

  std::vector<LogEntry> pending_entries_;
  std::mutex entries_mutex_;

  int kill_count_ = 0;
  int death_count_ = 0;
  bool log_closed_ = false;
};

} // namespace exile::game