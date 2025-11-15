#ifndef BACKEND_GENERICREPOSITORY_THREADPOOL_H_
#define BACKEND_GENERICREPOSITORY_THREADPOOL_H_

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
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();
    void waitAll();

  private:
    void enqueueTask(std::function<void()> task) override;

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable done_condition_;
    bool stop_;
    size_t active_tasks_ = 0;
};

#endif  // BACKEND_GENERICREPOSITORY_THREADPOOL_H_"
