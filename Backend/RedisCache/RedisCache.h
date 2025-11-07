#ifndef BACKEND_REDISCACHE_REDISCACHE_H_
#define BACKEND_REDISCACHE_REDISCACHE_H_

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <sw/redis++/redis++.h>
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "ICacheService.h"

class RedisCache : public ICacheService {
 public:
  using json = nlohmann::json;

  static RedisCache& instance();
  void clearCache();
  void incr(const std::string& key) override;
  void remove(const std::string& key) override;
  bool exists(const std::string& key) override;
  void clearPrefix(const std::string& prefix);

  void setPipelines(const std::vector<std::string>& keys, const std::vector<nlohmann::json>& results,
                    std::chrono::minutes ttl = std::chrono::minutes(30));

  void set(const std::string& key, const nlohmann::json& value,
           std::chrono::milliseconds ttl = std::chrono::minutes(30)) override;

  std::optional<nlohmann::json> get(const std::string& key) override;

 private:
  std::unique_ptr<sw::redis::Redis> redis_;
  std::mutex init_mutex_;

  // template <typename T>
  // std::string buildEntityKey(const T& entity, std::string table_name) const;

  // template <typename T>
  // long long getEntityId(const T& entity) const;

  sw::redis::Redis& getRedis();

  int getTtlWithJitter(std::chrono::seconds ttl);

  void logError(const std::string& action, const std::string& key,
                const std::exception& e);

  RedisCache() = default;
  RedisCache(const RedisCache&) = delete;
  RedisCache& operator=(const RedisCache&) = delete;
};


#endif  // BACKEND_REDISCACHE_REDISCACHE_H_
