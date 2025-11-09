#ifndef MOCKCACHE_H
#define MOCKCACHE_H

#include <vector>
#include <string>
#include <unordered_map>

#include "interfaces/ICacheService.h"

class MockCache : public ICacheService {
    std::unordered_map<std::string, int> mp;
    std::unordered_map<std::string, nlohmann::json> cache;
  public:

    void incr(const std::string& key) override {
      mp[key]++;
    }

    void remove(const std::string& key) override {
      mp[key]++;
      cache.erase(key);
    }

    bool exists(const std::string& key) override {
      auto it = cache.find(key);
      if(it == cache.end()) return false;
      return true;
    }

    int set_calls = 0;
    int set_pipeline_calls = 0;

    std::optional<nlohmann::json> get(const std::string& key) override {
      auto it = cache.find(key);
      if(it == cache.end()) return std::nullopt;
      return it->second;
    }

    void set(const std::string& key, const nlohmann::json& value,
             std::chrono::milliseconds ttl = std::chrono::hours(24)) override {
      LOG_INFO("Key to set: {}", key);
      ++set_calls;
      cache[key] = value;
      mp[key]++;
    }

    int getCalls(const std::string& key) {
      return mp[key];
    }

    void setPipelines(const std::vector<std::string>& keys, const std::vector<nlohmann::json>& results,
                      std::chrono::minutes ttl = std::chrono::minutes(30)) override {
      ++set_pipeline_calls;
    }
};

#endif // MOCKCACHE_H
