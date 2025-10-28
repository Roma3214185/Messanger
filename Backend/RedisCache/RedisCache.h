#ifndef BACKEND_REDISCACHE_REDISCACHE_H_
#define BACKEND_REDISCACHE_REDISCACHE_H_

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <sw/redis++/redis++.h>
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"

class RedisCache {
 public:
  using json = nlohmann::json;

  static RedisCache& instance() {
    static RedisCache inst;
    return inst;
  }

  void clearCache();

  template <typename T>
  void saveEntities(const std::vector<T>& results, std::string table_name,
                    std::chrono::minutes ttl = std::chrono::minutes(30)) {
    try {
      auto pipe = redis_->pipeline();

      for (const auto& entity : results) {
        std::string entityKey = buildEntityKey(entity, table_name);
        auto value = serialize(entity);
        redis_->set(entityKey, value, ttl);
      }

      pipe.exec();
      LOG_INFO("Saved {} entities in Redis pipeline", results.size());

    } catch (const sw::redis::Error& e) {
      LOG_ERROR("Redis pipeline failed: {}", e.what());
    }
  }

  template <typename T>
  void saveEntity(const T& entity, std::string table_name,
                  std::chrono::minutes ttl = std::chrono::minutes(30)) {
    std::string entity_key = buildEntityKey(entity, table_name);
    auto value = serialize(entity);
    set(entity_key, value, ttl);
  }

  template <typename T, typename Duration = std::chrono::hours>
  void set(const std::string& key, const T& value,
           Duration ttl = std::chrono::hours(24)) {
    try {
      const std::string serialized_enity = serialize(value);
      LOG_INFO("set for key '{}' value: '{}'", key, serialized_enity);
      const auto ttlSeconds = getTtlWithJitter(ttl);

      getRedis().set(key, serialized_enity);
      getRedis().expire(key, std::chrono::seconds(ttlSeconds));

    } catch (const std::exception& e) {
      logError("set", key, e);
    }
  }

  void incr(const std::string& key);

  template <typename T>
  std::optional<T> get(const std::string& key) {
    try {
      auto value = getRedis().get(key);
      if (value) {
        LOG_INFO("For key '{}' finded cashe", key);
        return deserialize<T>(*value);
      }
    } catch (const std::exception& e) {
      logError("get", key, e);
    }
    return std::nullopt;
  }

  void remove(const std::string& key);
  void clearPrefix(const std::string& prefix);
  bool exists(const std::string& key);

 private:
  std::unique_ptr<sw::redis::Redis> redis_;
  std::mutex init_mutex_;

  RedisCache() = default;

  template <typename T>
  std::string buildEntityKey(const T& entity, std::string table_name) const {
    return "entity_cache:" + table_name + ":" +
           std::to_string(getEntityId(entity));
  }

  template <typename T>
  long long getEntityId(const T& entity) const {
    return entity.id;
  }

  sw::redis::Redis& getRedis() {
    if (!redis_) {
      std::scoped_lock lock(init_mutex_);
      if (!redis_) {
        try {
          redis_ = std::make_unique<sw::redis::Redis>("tcp://127.0.0.1:6379");
        } catch (const std::exception& e) {
          throw std::runtime_error(std::string("Redis init failed: ") +
                                   e.what());
        }
      }
    }
    return *redis_;
  }

  template <typename T>
  std::string serialize(const T& value) {
    if constexpr (std::is_arithmetic_v<T>) {
      return std::to_string(value);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return value;
    } else {
      return json(value).dump();
    }
  }

  template <typename T>
  T deserialize(const std::string& str) {
    if constexpr (std::is_arithmetic_v<T>) {
      T val{};
      std::istringstream(str) >> val;
      return val;
    } else if constexpr (std::is_same_v<T, std::string>) {
      return str;
    } else {
      return json::parse(str).get<T>();
    }
  }

  template <typename Duration>
  int getTtlWithJitter(Duration ttl) {
    const int baseSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(ttl).count();
    const int jitter = (std::rand() % 61 - 30) * 60;  // +-30 minutes
    return std::max(baseSeconds + jitter, 300);
  }

  void logError(const std::string& action, const std::string& key,
                const std::exception& e) {
    LOG_ERROR("'{}' ( '{}' ): '{}'", action, key, e.what());
  }

  RedisCache(const RedisCache&) = delete;
  RedisCache& operator=(const RedisCache&) = delete;
};

#endif  // BACKEND_REDISCACHE_REDISCACHE_H_
