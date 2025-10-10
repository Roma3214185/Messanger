#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <mutex>
#include <unordered_map>

using json = nlohmann::json;

struct RateLimiter {
    std::unordered_map<std::string, std::pair<int, std::chrono::steady_clock::time_point>> map;
    std::mutex m;
    int maxRequests;
    std::chrono::seconds window;

    RateLimiter(int maxReq = 300, std::chrono::seconds win = std::chrono::seconds(900)) : maxRequests(maxReq), window(win) {}

    bool allow(const std::string& ip) {
        std::lock_guard<std::mutex> lk(m);
        auto now = std::chrono::steady_clock::now();
        auto &entry = map[ip];
        if (entry.second == std::chrono::steady_clock::time_point()) {
            entry.first = 1;
            entry.second = now;
            return true;
        }
        auto elapsed = now - entry.second;
        if (elapsed > window) {
            entry.first = 1;
            entry.second = now;
            return true;
        }
        if (entry.first < maxRequests) {
            entry.first++;
            return true;
        }
        return false;
    }
};

#endif // RATELIMITER_H
