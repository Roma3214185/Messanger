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

  template <typename T>
  void saveEntities(const std::vector<T>& results, std::string table_name,
                    std::chrono::minutes ttl = std::chrono::minutes(30));

  template <typename T>
  void saveEntity(const T& entity, std::string table_name,
                  std::chrono::minutes ttl = std::chrono::minutes(30));

  template <typename T, typename Duration = std::chrono::hours>
  void set(const std::string& key, const T& value,
           Duration ttl = std::chrono::hours(24));

  template <typename T>
  std::optional<T> get(const std::string& key);

 private:
  std::unique_ptr<sw::redis::Redis> redis_;
  std::mutex init_mutex_;

  template <typename T>
  std::string buildEntityKey(const T& entity, std::string table_name) const;

  template <typename T>
  long long getEntityId(const T& entity) const;

  sw::redis::Redis& getRedis();

  template <typename T>
  std::string serialize(const T& value);

  template <typename T>
  T deserialize(const std::string& str);

  template <typename Duration>
  int getTtlWithJitter(Duration ttl);

  void logError(const std::string& action, const std::string& key,
                const std::exception& e);

  RedisCache() = default;
  RedisCache(const RedisCache&) = delete;
  RedisCache& operator=(const RedisCache&) = delete;
};

#include "RedisCache.inl"

#endif  // BACKEND_REDISCACHE_REDISCACHE_H_
