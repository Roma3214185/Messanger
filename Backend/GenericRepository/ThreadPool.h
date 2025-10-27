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
  explicit ThreadPool(size_t numThreads) {
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

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      stop_ = true;
    }
    condition_.notify_all();
    for (auto& worker : workers_) worker.join();
  }

  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      if (stop_) throw std::runtime_error("enqueue on stopped ThreadPool");
      tasks_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return task->get_future();
  }

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

#endif  // BACKEND_GENERICREPOSITORY_THREADPOOL_H_"
