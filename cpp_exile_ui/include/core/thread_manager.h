#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace exile::core {

using ThreadTask = std::function<void()>;

class ThreadManager {
public:
  ThreadManager();
  ~ThreadManager();

  bool start_secondary_thread();
  void stop_secondary_thread();
  bool start_ocr_thread();
  void stop_ocr_thread();

  void post_to_secondary(ThreadTask task);
  void post_to_ocr(ThreadTask task);
  void post_to_main(ThreadTask task);

  bool send_message_to_secondary(const std::string& message);
  void register_message_handler(std::function<void(const std::string&)> handler);

  bool is_secondary_running() const { return secondary_running_; }
  bool is_ocr_running() const { return ocr_running_; }

  void process_main_queue();

private:
  void secondary_loop();
  void ocr_loop();

  std::thread secondary_thread_;
  std::thread ocr_thread_;
  std::atomic<bool> secondary_running_{false};
  std::atomic<bool> ocr_running_{false};

  std::mutex secondary_mutex_;
  std::condition_variable secondary_cv_;
  std::vector<ThreadTask> secondary_queue_;

  std::mutex ocr_mutex_;
  std::condition_variable ocr_cv_;
  std::vector<ThreadTask> ocr_queue_;

  std::mutex main_mutex_;
  std::vector<ThreadTask> main_queue_;

  std::function<void(const std::string&)> message_handler_;
};

} // namespace exile::core