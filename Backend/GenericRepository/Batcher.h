#ifndef BACKEND_GENERICREPOSITORY_BATCHER_H_
#define BACKEND_GENERICREPOSITORY_BATCHER_H_

#include <utility>
#include <vector>

#include "../DebugProfiling/Debug_profiling.h"
#include "GenericRepository/GenericRepository.h"

template <typename T>
class SaverBatcher {
 private:
  std::vector<T> batcher_;
  std::mutex mtx_;
  std::condition_variable cv_;
  GenericRepository& rep_;
  ThreadPool pool_;
  const int batch_size_;
  const std::chrono::milliseconds flush_interval_;

  std::atomic<bool> running_{true};
  std::thread flush_thread_;

 public:
  SaverBatcher(
      GenericRepository& repository, int batch_size = 500,
      std::chrono::milliseconds interval = std::chrono::milliseconds(100),
      int thread_pool_size = 4)
      : rep_(repository),
        pool_(thread_pool_size),
        batch_size_(batch_size),
        flush_interval_(interval) {
    flush_thread_ = std::thread([this]() { flushLoop(); });
  }

  ~SaverBatcher() {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      running_.store(false);
    }
    cv_.notify_one();
    if (flush_thread_.joinable()) flush_thread_.join();

    flush();
    LOG_INFO("~SaverBatcher()");
  }

  void saveEntity(T& entity) {
    PROFILE_SCOPE("Batcher::SaveEntity");
    std::unique_lock<std::mutex> lock(mtx_);
    batcher_.emplace_back(std::move(entity));
  }

  void flush() {
    std::vector<T> local_batch;
    {
      std::unique_lock<std::mutex> lock(mtx_);
      if (batcher_.empty()) return;

      local_batch = std::move(batcher_);
      batcher_.clear();
    }

    pool_.enqueue([this, local_batch = std::move(local_batch)]() mutable {
      try {
        rep_.save(local_batch);
      } catch (...) {
        LOG_ERROR("Error saving batch");
      }
    });
  }

 private:
  void flushLoop() {
    std::unique_lock<std::mutex> lock(mtx_);
    while (running_.load()) {
      cv_.wait_for(lock, flush_interval_,
                   [this]() { return !running_.load(); });
      lock.unlock();
      flush();
      lock.lock();
    }
  }

  SaverBatcher(const SaverBatcher&) = delete;
  SaverBatcher& operator=(const SaverBatcher&) = delete;
};

template <typename T>
class DeleterBatcher {
 private:
  std::vector<T> batcher_;
  std::mutex mtx_;
  std::condition_variable cv_;
  GenericRepository& rep_;
  ThreadPool pool_;
  const int batch_size_;
  const std::chrono::milliseconds flush_interval_;

  std::atomic<bool> running_{true};
  std::thread flush_thread_;

 public:
  DeleterBatcher(
      GenericRepository& repository, int batch_size = 500,
      std::chrono::milliseconds interval = std::chrono::milliseconds(100),
      int thread_pool_size = 4)
      : rep_(repository),
        pool_(thread_pool_size),
        batch_size_(batch_size),
        flush_interval_(interval) {
    flush_thread_ = std::thread([this]() { flushLoop(); });
  }

  ~DeleterBatcher() {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      running_.store(false);
    }
    cv_.notify_one();
    if (flush_thread_.joinable()) flush_thread_.join();

    flush();
    LOG_INFO("~DeleterBatcher()");
  }

  void deleteEntity(T entity) {
    PROFILE_SCOPE("Batcher::DeleteEntity");
    std::vector<T> local_batch;
    {
      std::unique_lock<std::mutex> lock(mtx_);
      batcher_.emplace_back(std::move(entity));

      if (batcher_.size() >= batch_size_) {
        local_batch = std::move(batcher_);
        batcher_.clear();
      }
    }

    if (!local_batch.empty()) {
      pool_.enqueue([this, local_batch = std::move(local_batch)]() mutable {
        try {
          rep_.deleteBatch(local_batch);
        } catch (...) {
          LOG_ERROR("Error saving batch");
        }
      });
    }
  }

  void flush() {
    std::vector<T> local_batch;
    {
      std::unique_lock<std::mutex> lock(mtx_);
      if (batcher_.empty()) return;

      local_batch = std::move(batcher_);
      batcher_.clear();
    }

    pool_.enqueue([this, local_batch = std::move(local_batch)]() mutable {
      try {
        rep_.deleteBatch(local_batch);
      } catch (...) {
        LOG_ERROR("Error delete batch");
      }
    });
  }

 private:
  void flushLoop() {
    std::unique_lock<std::mutex> lock(mtx_);
    while (running_.load()) {
      cv_.wait_for(lock, flush_interval_,
                   [this]() { return !running_.load(); });
      lock.unlock();
      flush();
      lock.lock();
    }
  }

  DeleterBatcher(const DeleterBatcher&) = delete;
  DeleterBatcher& operator=(const DeleterBatcher&) = delete;
};
template <typename T>
class Batcher {
  SaverBatcher<T>& saverBatcher;
  DeleterBatcher<T>& deleterBatcher;

 public:
  Batcher(SaverBatcher<T>& saverBatcher, DeleterBatcher<T>& deleterBatcher)
      : saverBatcher(saverBatcher), deleterBatcher(deleterBatcher) {}

  ~Batcher() { LOG_INFO("~Batcher()"); }

  void save(T& entity) {
    PROFILE_SCOPE("Batcher::save");
    saverBatcher.saveEntity(entity);
  }

  void deleteEntity(T& entity) { deleterBatcher.deleteEntity(entity); }
};

#endif  // BACKEND_GENERICREPOSITORY_BATCHER_H_
