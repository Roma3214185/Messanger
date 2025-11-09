#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) {
  for (size_t i = 0; i < numThreads; ++i) {
    workers_.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

          if (stop_ && tasks_.empty()) return;

          task = std::move(tasks_.front());
          tasks_.pop();
          ++active_tasks_;
        }

        task();

        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          --active_tasks_;
          if (tasks_.empty() && active_tasks_ == 0) {
            done_condition_.notify_all();
          }
        }
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    stop_ = true;
  }
  condition_.notify_all();
  for (auto& worker : workers_) worker.join();
}
