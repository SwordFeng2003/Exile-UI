#include "game/economy_fetcher.h"
#include "core/config_manager.h"
#include "core/log_manager.h"
#include <fstream>
#include <chrono>

namespace exile::game {

EconomyFetcher::EconomyFetcher() {
  curl_ = curl_easy_init();
}

EconomyFetcher::~EconomyFetcher() {
  if (curl_) curl_easy_cleanup(curl_);
}

bool EconomyFetcher::fetch_currency_prices(const std::string& league) {
  std::string url = "https://poe.ninja/api/data/currencyoverview?league=" +
                    league + "&type=Currency&language=en";
  try {
    auto data = get_json(url);

    if (data.contains("lines")) {
      for (const auto& line : data["lines"]) {
        EconomyPrice price;
        price.name = line.value("currencyTypeName", "Unknown");
        price.chaos_value = line.value("chaosEquivalent", 0.0f);
        price.divine_value = line.value("divineEquivalent", 0.0f);

        if (line.contains("lowConfidencePaySparkLine")) {
          for (const auto& v : line["lowConfidencePaySparkLine"]["data"]) {
            price.trend.push_back(v.get<float>());
          }
        }

        prices_[price.name] = price;
      }
    }
    return true;
  } catch (const std::exception& e) {
    core::LogManager::instance().error("Failed to fetch currency prices: {}", e.what());
    return false;
  }
}

bool EconomyFetcher::fetch_item_prices(const std::string& league, const std::string& item_type) {
  std::string url = "https://poe.ninja/api/data/itemoverview?league=" +
                    league + "&type=" + item_type + "&language=en";
  try {
    auto data = get_json(url);
    if (data.contains("lines")) {
      for (const auto& line : data["lines"]) {
        EconomyPrice price;
        price.name = line.value("name", "Unknown");
        price.chaos_value = line.value("chaosValue", 0.0f);
        price.divine_value = line.value("divineValue", 0.0f);
        prices_[price.name] = price;
      }
    }
    return true;
  } catch (const std::exception& e) {
    core::LogManager::instance().error("Failed to fetch item prices: {}", e.what());
    return false;
  }
}

bool EconomyFetcher::fetch_bulk_exchange(const std::string& league, const std::string& have,
                                          const std::string& want) {
  std::string url = "https://poe.ninja/api/data/bulk?league=" + league +
                    "&have=" + have + "&want=" + want;
  try {
    auto data = get_json(url);
    return !data.empty();
  } catch (const std::exception& e) {
    return false;
  }
}

const EconomyPrice* EconomyFetcher::get_price(const std::string& item) const {
  auto it = prices_.find(item);
  if (it != prices_.end()) return &it->second;
  return nullptr;
}

bool EconomyFetcher::needs_update(const std::string& type, int max_age_minutes) const {
  auto it = update_timestamps_.find(type);
  if (it == update_timestamps_.end()) return true;

  auto now = std::chrono::duration_cast<std::chrono::minutes>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  return (now - static_cast<int64_t>(it->second)) > max_age_minutes;
}

void EconomyFetcher::set_update_timestamp(const std::string& type) {
  auto now = std::chrono::duration_cast<std::chrono::minutes>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  update_timestamps_[type] = static_cast<uint64_t>(now);
}

std::string EconomyFetcher::http_get(const std::string& url) {
  if (!curl_) return "";

  std::string response;
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl_, CURLOPT_USERAGENT, "ExileUI/1.0");

  CURLcode res = curl_easy_perform(curl_);
  if (res != CURLE_OK) {
    core::LogManager::instance().error("HTTP GET failed: {}", curl_easy_strerror(res));
    return "";
  }

  return response;
}

std::string EconomyFetcher::http_post(const std::string& url, const std::string& body,
                                       const std::string& content_type) {
  if (!curl_) return "";

  std::string response;
  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, ("Content-Type: " + content_type).c_str());

  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);

  CURLcode res = curl_easy_perform(curl_);
  curl_slist_free_all(headers);

  if (res != CURLE_OK) return "";
  return response;
}

nlohmann::json EconomyFetcher::get_json(const std::string& url) {
  auto text = http_get(url);
  if (text.empty()) return nlohmann::json{};
  return nlohmann::json::parse(text);
}

bool EconomyFetcher::download_file(const std::string& url, const std::filesystem::path& dest) {
  auto text = http_get(url);
  if (text.empty()) return false;

  std::ofstream file(dest, std::ios::binary);
  if (!file.is_open()) return false;
  file.write(text.c_str(), text.size());
  return true;
}

size_t EconomyFetcher::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t total = size * nmemb;
  auto* str = static_cast<std::string*>(userp);
  str->append(static_cast<char*>(contents), total);
  return total;
}

} // namespace exile::game