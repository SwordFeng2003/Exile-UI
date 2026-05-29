#include "core/thread_manager.h"

namespace exile::core {

ThreadManager::ThreadManager() {}

ThreadManager::~ThreadManager() {
  stop_secondary_thread();
  stop_ocr_thread();
}

bool ThreadManager::start_secondary_thread() {
  if (secondary_running_) return true;
  secondary_running_ = true;
  secondary_thread_ = std::thread(&ThreadManager::secondary_loop, this);
  return true;
}

void ThreadManager::stop_secondary_thread() {
  secondary_running_ = false;
  secondary_cv_.notify_all();
  if (secondary_thread_.joinable()) secondary_thread_.join();
}

bool ThreadManager::start_ocr_thread() {
  if (ocr_running_) return true;
  ocr_running_ = true;
  ocr_thread_ = std::thread(&ThreadManager::ocr_loop, this);
  return true;
}

void ThreadManager::stop_ocr_thread() {
  ocr_running_ = false;
  ocr_cv_.notify_all();
  if (ocr_thread_.joinable()) ocr_thread_.join();
}

void ThreadManager::post_to_secondary(ThreadTask task) {
  {
    std::lock_guard<std::mutex> lock(secondary_mutex_);
    secondary_queue_.push_back(std::move(task));
  }
  secondary_cv_.notify_one();
}

void ThreadManager::post_to_ocr(ThreadTask task) {
  {
    std::lock_guard<std::mutex> lock(ocr_mutex_);
    ocr_queue_.push_back(std::move(task));
  }
  ocr_cv_.notify_one();
}

void ThreadManager::post_to_main(ThreadTask task) {
  std::lock_guard<std::mutex> lock(main_mutex_);
  main_queue_.push_back(std::move(task));
}

bool ThreadManager::send_message_to_secondary(const std::string& message) {
  post_to_secondary([this, message]() {
    if (message_handler_) {
      message_handler_(message);
    }
  });
  return true;
}

void ThreadManager::register_message_handler(std::function<void(const std::string&)> handler) {
  message_handler_ = std::move(handler);
}

void ThreadManager::process_main_queue() {
  std::vector<ThreadTask> tasks;
  {
    std::lock_guard<std::mutex> lock(main_mutex_);
    tasks.swap(main_queue_);
  }
  for (auto& task : tasks) {
    task();
  }
}

void ThreadManager::secondary_loop() {
  while (secondary_running_) {
    std::vector<ThreadTask> tasks;
    {
      std::unique_lock<std::mutex> lock(secondary_mutex_);
      secondary_cv_.wait(lock, [this]() {
        return !secondary_running_ || !secondary_queue_.empty();
      });
      if (!secondary_running_) break;
      tasks.swap(secondary_queue_);
    }
    for (auto& task : tasks) {
      if (!secondary_running_) break;
      task();
    }
  }
}

void ThreadManager::ocr_loop() {
  while (ocr_running_) {
    std::vector<ThreadTask> tasks;
    {
      std::unique_lock<std::mutex> lock(ocr_mutex_);
      ocr_cv_.wait(lock, [this]() {
        return !ocr_running_ || !ocr_queue_.empty();
      });
      if (!ocr_running_) break;
      tasks.swap(ocr_queue_);
    }
    for (auto& task : tasks) {
      if (!ocr_running_) break;
      task();
    }
  }
}

} // namespace exile::core