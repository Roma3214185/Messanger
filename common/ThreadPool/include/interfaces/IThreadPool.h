#ifndef BACKEND_GENERICREPOSITORY_ITHREADPOOL_H_
#define BACKEND_GENERICREPOSITORY_ITHREADPOOL_H_

#include <functional>
#include <future>

struct IThreadPool {
  virtual ~IThreadPool() = default;

  template <typename F> auto enqueue(F &&f) -> std::future<decltype(f())> {
    using ReturnType = decltype(f());
    auto task =
        std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));
    std::future<ReturnType> result = task->get_future();
    enqueueTask([task]() { (*task)(); });

    return result;
  }

protected:
  virtual void enqueueTask(std::function<void()> task) = 0;
};

#endif // BACKEND_GENERICREPOSITORY_ITHREADPOOL_H_
