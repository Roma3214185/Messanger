#ifndef BACKEND_REDISCACHE_REDISCACHE_H_
#define BACKEND_REDISCACHE_REDISCACHE_H_

#include <sw/redis++/redis.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include "interfaces/ICacheService.h"

class RedisCache : public ICacheService {
 public:
  static RedisCache &instance();
  RedisCache(const RedisCache &) = delete;
  RedisCache &operator=(const RedisCache &) = delete;
  RedisCache(RedisCache &&) = delete;
  RedisCache &operator=(RedisCache &&) = delete;

  void incr(const std::string &key) override;
  void remove(const std::string &key) override;
  void clearCache() override;

  void setPipelines(const std::vector<std::string> &keys, const std::vector<std::string> &results,
                    std::chrono::seconds ttl = std::chrono::seconds(5)) override;

  void set(const std::string &key, const std::string &value,
           std::chrono::seconds ttl = std::chrono::seconds(5)) override;

  std::optional<std::string> get(const std::string &key) override;

 private:
  std::unique_ptr<sw::redis::Redis> redis_;
  std::mutex init_mutex_;

  sw::redis::Redis &getRedis();
  int getTtlWithJitter(std::chrono::seconds ttl);

  RedisCache() = default;
  ~RedisCache() = default;
};

#endif  // BACKEND_REDISCACHE_REDISCACHE_H_
