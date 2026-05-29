#include "modules/exchange.h"
#include "core/config_manager.h"
#include "core/language_manager.h"
#include "core/application.h"
#include "game/clipboard_handler.h"
#include "game/input_simulator.h"
#include "game/economy_fetcher.h"
#include "ui/widgets.h"
#include "ui/theme.h"
#include <imgui.h>

namespace exile::modules {

void Exchange::initialize() {
  auto& cfg = core::Application::instance().config_manager();
  active_button_ = cfg.read<std::string>("Exchange", "active-button", "");
  row_separator_ = cfg.read<bool>("Exchange", "row-separator", true);
}

void Exchange::shutdown() {
}

void Exchange::render_overlay() {
  if (!overlay_visible_) return;

  auto& lang = core::Application::instance().language_manager();

  ImGui::Begin(lang.translate("exchange").c_str(), &overlay_visible_);

  ImGui::Text("%s: %d", lang.translate("total trades").c_str(), total_trades_);
  ImGui::SameLine();
  ImGui::Text("| %s: %.1fc", lang.translate("earnings").c_str(), total_earnings_);

  ImGui::Separator();

  if (ImGui::Button(lang.translate("process whisper").c_str())) {
    auto whisper = game::ClipboardHandler::instance().get_text();
    if (!whisper.empty()) process_whisper(whisper);
  }

  ImGui::SameLine();
  if (ImGui::Button("Kick")) on_kick_buyer();

  if (ImGui::Button(lang.translate("thanks").c_str())) on_thanks_message();
  ImGui::SameLine();
  if (ImGui::Button(lang.translate("invite").c_str())) on_invite_party();
  ImGui::SameLine();
  if (ImGui::Button(lang.translate("trade").c_str())) on_trade_confirm();

  ImGui::Separator();

  if (ImGui::BeginTable("##offers_table", 4,
      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
    ImGui::TableSetupColumn(lang.translate("buyer").c_str());
    ImGui::TableSetupColumn(lang.translate("item").c_str());
    ImGui::TableSetupColumn(lang.translate("price").c_str());
    ImGui::TableSetupColumn(lang.translate("status").c_str());
    ImGui::TableHeadersRow();

    for (auto& offer : offers_) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%s", offer.player_name.c_str());
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%s", offer.item_name.c_str());
      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%d %s", offer.price_amount, offer.price_currency.c_str());
      ImGui::TableSetColumnIndex(3);
      if (offer.sold) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Sold");
      } else {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Pending");
      }
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void Exchange::show_overlay() { overlay_visible_ = true; }
void Exchange::hide_overlay() { overlay_visible_ = false; }

void Exchange::process_whisper(const std::string& whisper_text) {
  ExchangeOffer offer;
  offer.whisper_text = whisper_text;
  offer.timestamp = GetTickCount64();

  auto first_colon = whisper_text.find(':');
  if (first_colon != std::string::npos) {
    auto name_start = whisper_text.rfind('@', first_colon);
    if (name_start != std::string::npos) {
      offer.player_name = whisper_text.substr(name_start + 1, first_colon - name_start - 1);
    }
  }

  offers_.push_back(offer);
}

void Exchange::handle_sale(const std::string& player_name) {
  for (auto& offer : offers_) {
    if (offer.player_name == player_name) {
      offer.sold = true;
      total_earnings_ += offer.price_value;
      total_trades_++;
      trade_history_.push_back(offer);
    }
  }
}

void Exchange::on_bulk_trade(const std::string& whisper_text) {
  bulk_mode_ = true;
  process_whisper(whisper_text);
}

void Exchange::on_single_trade(const std::string& whisper_text) {
  bulk_mode_ = false;
  process_whisper(whisper_text);
}

void Exchange::on_invite_party() {
  game::InputSimulator::send_enter();
  Sleep(100);
  game::InputSimulator::send_text("/invite ");
  if (!offers_.empty()) {
    game::InputSimulator::send_text(offers_.back().player_name);
  }
  game::InputSimulator::send_enter();
}

void Exchange::on_trade_confirm() {
  game::InputSimulator::send_enter();
  Sleep(100);
  game::InputSimulator::send_text("/tradewith ");
  if (!offers_.empty()) {
    game::InputSimulator::send_text(offers_.back().player_name);
  }
  game::InputSimulator::send_enter();
}

void Exchange::on_kick_buyer() {
  game::InputSimulator::send_enter();
  Sleep(100);
  game::InputSimulator::send_text("/kick ");
  if (!offers_.empty()) {
    game::InputSimulator::send_text(offers_.back().player_name);
  }
  game::InputSimulator::send_enter();
}

void Exchange::on_ignore_buyer(const std::string& name) {
  game::InputSimulator::send_enter();
  Sleep(100);
  game::InputSimulator::send_text("/ignore " + name);
  game::InputSimulator::send_enter();
}

void Exchange::on_thanks_message() {
  game::InputSimulator::send_enter();
  Sleep(100);
  game::InputSimulator::send_text("ty");
  game::InputSimulator::send_enter();
  if (!offers_.empty()) {
    handle_sale(offers_.back().player_name);
  }
}

void Exchange::on_out_of_stock() {
}

void Exchange::on_override_ignore() {
}

std::string Exchange::read_incoming_whisper() {
  return "";
}

std::string Exchange::read_outgoing_whisper() {
  return "";
}

void Exchange::move_row_up(int index) {
  if (index > 0 && index < static_cast<int>(offers_.size())) {
    std::swap(offers_[index], offers_[index - 1]);
  }
}

void Exchange::move_row_down(int index) {
  if (index >= 0 && index + 1 < static_cast<int>(offers_.size())) {
    std::swap(offers_[index], offers_[index + 1]);
  }
}

void Exchange::clear_trade_data() {
  offers_.clear();
}

} // namespace exile::modules