#ifndef BACKEND_APIGATEWAY_SRC_HEADERS_RATELIMITER_H_
#define BACKEND_APIGATEWAY_SRC_HEADERS_RATELIMITER_H_

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

#include "interfaces/iratelimiter.h"

class RateLimiter : public IRateLimiter {
 public:

  struct Entry {
      int                                   requests = 0;
      std::chrono::steady_clock::time_point windowStart{};
  };

  using IP = std::string;
  using EntriesMap = std::unordered_map<IP, Entry>;


  explicit RateLimiter(int maxReq = 300, std::chrono::seconds win = std::chrono::seconds(900))
      : maxRequests(maxReq), window(win) {}

  bool allow(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto                        now = std::chrono::steady_clock::now();

    auto& entry = entries_[ip];
    if (isNewWindow(entry, now)) {
      resetEntry(entry, now);
      return true;
    }

    if (entry.requests < maxRequests) {
      entry.requests++;
      return true;
    }

    return false;
  }

 private:
  EntriesMap           entries_;
  std::mutex           mutex_;
  int                  maxRequests;
  std::chrono::seconds window;

  bool isNewWindow(const Entry& entry, const std::chrono::steady_clock::time_point& now) const {
    return entry.windowStart == std::chrono::steady_clock::time_point{} ||
           (now - entry.windowStart) > window;
  }

  void resetEntry(Entry& entry, const std::chrono::steady_clock::time_point& now) {
    entry.requests    = 1;
    entry.windowStart = now;
  }
};

#endif  // BACKEND_APIGATEWAY_SRC_HEADERS_RATELIMITER_H_
