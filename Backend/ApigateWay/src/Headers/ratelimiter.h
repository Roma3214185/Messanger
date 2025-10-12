#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <mutex>
#include <unordered_map>
#include <chrono>
#include <string>

using namespace std::chrono;

class RateLimiter
{
public:

    using IP = std::string;

    struct Entry {
        int requests = 0;
        steady_clock::time_point windowStart{};
    };

    using EntriesMap = std::unordered_map<IP, Entry>;

    explicit RateLimiter(int maxReq = 300, seconds win = seconds(900))
        : maxRequests(maxReq), window(win) {}

    bool allow(const std::string& ip) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = steady_clock::now();

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

    EntriesMap entries_;
    std::mutex mutex_;
    int maxRequests;
    seconds window;

    bool isNewWindow(const Entry& entry, const steady_clock::time_point& now) const {
        return entry.windowStart == steady_clock::time_point{} || (now - entry.windowStart) > window;
    }

    void resetEntry(Entry& entry, const std::chrono::steady_clock::time_point& now) {
        entry.requests = 1;
        entry.windowStart = now;
    }
};

#endif // RATELIMITER_H
