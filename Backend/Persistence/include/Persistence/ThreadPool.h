#ifndef BACKEND_GENERICREPOSITORY_THREADPOOL_H_
#define BACKEND_GENERICREPOSITORY_THREADPOOL_H_

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <vector>
#include <utility>

class ThreadPool {
 public:
  explicit ThreadPool(size_t numThreads);
  ~ThreadPool();

  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result<F, Args...>::type>;

  void waitAll() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    done_condition_.wait(lock,
                       [this] { return tasks_.empty() && active_tasks_ == 0; });
  }

 private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;

  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::condition_variable done_condition_;
  bool stop_ = false;
  size_t active_tasks_ = 0;
};

#include "Persistence/inl/ThreadPool.inl"

#endif  // BACKEND_GENERICREPOSITORY_THREADPOOL_H_"
