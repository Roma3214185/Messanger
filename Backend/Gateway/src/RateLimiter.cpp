#include "ratelimiter.h"

RateLimiter::RateLimiter(int maxReq, std::chrono::seconds win) : maxRequests(maxReq), window(win) {}

bool RateLimiter::allow(const std::string &ip) {
  std::scoped_lock lock(mutex_);
  auto now = std::chrono::steady_clock::now();

  auto &entry = entries_[ip];
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

bool RateLimiter::isNewWindow(const Entry &entry, const std::chrono::steady_clock::time_point &now) const {
  return entry.windowStart == std::chrono::steady_clock::time_point{} || (now - entry.windowStart) > window;
}

void RateLimiter::resetEntry(Entry &entry, const std::chrono::steady_clock::time_point &now) {
  entry.requests = 1;
  entry.windowStart = now;
}
