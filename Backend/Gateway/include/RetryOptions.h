#ifndef RETRYOPTIONS_H
#define RETRYOPTIONS_H
// retry.h
#pragma once
#include <chrono>
#include <functional>
#include <optional>
#include <thread>

struct RetryOptions {
  int max_attempts = 3;
  std::chrono::milliseconds retry_delay{200};
  bool exponential_backoff = true;
  std::chrono::milliseconds per_attempt_timeout{2000};
};

template <typename Func>
auto retryInvoke(Func func, const RetryOptions &opts)
    -> std::optional<decltype(func())> {
  using namespace std::chrono;
  for (int attempt = 1; attempt <= opts.max_attempts; attempt++) {
    std::optional<decltype(func())> result;
    bool done = false;

    std::thread worker([&result, &done, &func]() mutable {
      auto r = func();
      result = std::make_optional(std::move(r));
      done = true;
    });

    auto start = steady_clock::now();
    while (!done && steady_clock::now() - start < opts.per_attempt_timeout) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    if (!done) {
      worker.detach();
    } else {
      worker.join();
    }

    if (result.has_value())
      return result;

    // no success - retry delay
    if (attempt < opts.max_attempts) {
      if (opts.exponential_backoff) {
        auto delay = opts.retry_delay * (1 << (attempt - 1));
        std::this_thread::sleep_for(delay);
      } else {
        std::this_thread::sleep_for(opts.retry_delay);
      }
    }
  }
  return std::nullopt;
}

#endif // RETRYOPTIONS_H
