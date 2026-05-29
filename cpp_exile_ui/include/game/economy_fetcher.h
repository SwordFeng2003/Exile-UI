#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace exile::game {

struct EconomyPrice {
  std::string name;
  float chaos_value = 0.0f;
  float divine_value = 0.0f;
  std::vector<float> trend;
  uint64_t last_updated = 0;
};

class EconomyFetcher {
public:
  EconomyFetcher();
  ~EconomyFetcher();

  bool fetch_currency_prices(const std::string& league);
  bool fetch_item_prices(const std::string& league, const std::string& item_type);
  bool fetch_bulk_exchange(const std::string& league, const std::string& have,
                          const std::string& want);

  const std::unordered_map<std::string, EconomyPrice>& prices() const { return prices_; }
  const EconomyPrice* get_price(const std::string& item) const;

  bool needs_update(const std::string& type, int max_age_minutes = 60) const;
  void set_update_timestamp(const std::string& type);

  std::string http_get(const std::string& url);
  std::string http_post(const std::string& url, const std::string& body,
                       const std::string& content_type = "application/json");

  nlohmann::json get_json(const std::string& url);
  bool download_file(const std::string& url, const std::filesystem::path& dest);

private:
  static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

  CURL* curl_ = nullptr;
  std::unordered_map<std::string, EconomyPrice> prices_;
  std::unordered_map<std::string, uint64_t> update_timestamps_;
};

} // namespace exile::game