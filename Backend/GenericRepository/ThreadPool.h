#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty())
                            return;

                        task = std::move(tasks.front());
                        tasks.pop();
                        ++activeTasks;
                    }

                    task();

                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        --activeTasks;
                        if (tasks.empty() && activeTasks == 0)
                            doneCondition.notify_all();
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& worker : workers)
            worker.join();
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return task->get_future();
    }

    void waitAll() {
        std::unique_lock<std::mutex> lock(queueMutex);
        doneCondition.wait(lock, [this] {
            return tasks.empty() && activeTasks == 0;
        });
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::condition_variable doneCondition;
    bool stop = false;
    size_t activeTasks = 0;
};

#endif // THREADPOOL_H
