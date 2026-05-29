#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace exile::modules {

struct ExchangeOffer {
  std::string whisper_text;
  std::string player_name;
  std::string item_name;
  std::string item_class;
  int price_amount = 0;
  std::string price_currency;
  float price_value = 0.0f;
  std::string tab_name;
  bool sold = false;
  bool processed = false;
  uint64_t timestamp = 0;
};

struct ExchangeListing {
  std::string item_name;
  std::string item_class;
  float price = 0.0f;
  std::string price_unit;
  int stock = 0;
  std::string tab;
  int priority = 0;
};

class Exchange {
public:
  void initialize();
  void shutdown();

  void render_overlay();
  void show_overlay();
  void hide_overlay();
  bool is_visible() const { return overlay_visible_; }

  void process_whisper(const std::string& whisper_text);
  void handle_sale(const std::string& player_name);

  void on_bulk_trade(const std::string& whisper_text);
  void on_single_trade(const std::string& whisper_text);
  void on_invite_party();
  void on_trade_confirm();
  void on_kick_buyer();
  void on_ignore_buyer(const std::string& name);
  void on_thanks_message();
  void on_out_of_stock();
  void on_override_ignore();

  float get_total_earnings() const { return total_earnings_; }
  int total_trades() const { return total_trades_; }

  std::string read_incoming_whisper();
  std::string read_outgoing_whisper();

  void set_active_button(const std::string& button) { active_button_ = button; }
  void set_row_separator(bool v) { row_separator_ = v; }

private:
  void move_row_up(int index);
  void move_row_down(int index);
  void clear_trade_data();

  std::vector<ExchangeOffer> offers_;
  std::vector<ExchangeListing> listings_;
  std::vector<ExchangeOffer> trade_history_;
  bool overlay_visible_ = false;
  bool bulk_mode_ = false;
  float total_earnings_ = 0.0f;
  int total_trades_ = 0;
  std::string active_button_;
  bool row_separator_ = true;

  nlohmann::json item_stats_;
  int tab_height_ = 0;
  int current_tab_ = 0;
  bool game_active_ = false;
};

} // namespace exile::modules