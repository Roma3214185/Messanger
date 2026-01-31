#ifndef BACKEND_APIGATEWAY_SRC_HEADERS_RATELIMITER_H_
#define BACKEND_APIGATEWAY_SRC_HEADERS_RATELIMITER_H_

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

#include "interfaces/IRateLimiter.h"

using TimePoint = std::chrono::steady_clock::time_point;

class RateLimiter : public IRateLimiter {
 public:
  struct Entry {  // todo: maybe extract
    int requests = 0;
    TimePoint windowStart{};
  };

  using IP = std::string;
  using EntriesMap = std::unordered_map<IP, Entry>;

  explicit RateLimiter(int maxReq = 300, std::chrono::seconds win = std::chrono::seconds{900});
  bool allow(const std::string &ip) override;

 private:
  bool isNewWindow(const Entry &entry, const std::chrono::steady_clock::time_point &now) const;
  void resetEntry(Entry &entry, const std::chrono::steady_clock::time_point &now);

  EntriesMap entries_;
  std::mutex mutex_;
  int maxRequests;
  std::chrono::seconds window;
};

#endif  // BACKEND_APIGATEWAY_SRC_HEADERS_RATELIMITER_H_
