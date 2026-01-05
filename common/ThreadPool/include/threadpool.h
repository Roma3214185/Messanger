#ifndef COMMON_THREADPOOL_H_
#define COMMON_THREADPOOL_H_

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include "interfaces/IThreadPool.h"

class ThreadPool : public IThreadPool {
 public:
  explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
  ~ThreadPool() override;
  ThreadPool(const ThreadPool&)            = delete;
  ThreadPool(ThreadPool&&)                 = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&)      = delete;

  void waitAll();

 private:
  void enqueueTask(std::function<void()> task) override;

  std::vector<std::thread>          workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex                        queue_mutex_;
  std::condition_variable           condition_;
  std::condition_variable           done_condition_;
  bool                              stop_{false};
  size_t                            active_tasks_ = 0;
};

#endif  // COMMON_THREADPOOL_H_
