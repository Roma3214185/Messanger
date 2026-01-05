#ifndef MOCKCACHE_H
#define MOCKCACHE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "interfaces/ICacheService.h"

class MockCache : public ICacheService {
  std::unordered_map<std::string, int>         mp;
  std::unordered_map<std::string, std::string> cache;

 public:
  void incr(const std::string& key) override { mp[key]++; }

  void remove(const std::string& key) override {
    mp[key]++;
    cache.erase(key);
  }

  bool exists(const std::string& key) {
    auto it = cache.find(key);
    return it != cache.end();
  }

  void clearCache() override {
    cache.clear();
    mp.clear();
  }

  void clearPrefix(const std::string& prefix) override {}

  int                        set_calls          = 0;
  int                        set_pipeline_calls = 0;
  std::optional<std::string> mock_get_string    = std::nullopt;
  bool                       get_should_fail    = false;

  std::optional<std::string> get(const std::string& key) override {
    auto it = cache.find(key);
    if (get_should_fail) return std::nullopt;
    if (mock_get_string) return mock_get_string;
    return it == cache.end() ? std::nullopt : std::make_optional(it->second);
  }

  void set(const std::string&   key,
           const std::string&   value,
           std::chrono::seconds ttl = std::chrono::hours(24)) override {
    ++set_calls;
    cache[key] = value;
    mp[key]++;
  }

  int getCalls(const std::string& key) { return mp[key]; }

  void setPipelines(const std::vector<std::string>& keys,
                    const std::vector<std::string>& results,
                    std::chrono::seconds            ttl = std::chrono::minutes(30)) override {
    ++set_pipeline_calls;
  }
};

#endif  // MOCKCACHE_H
