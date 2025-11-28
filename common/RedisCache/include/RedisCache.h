#ifndef BACKEND_REDISCACHE_REDISCACHE_H_
#define BACKEND_REDISCACHE_REDISCACHE_H_

#include <sw/redis++/redis++.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "interfaces/ICacheService.h"

class RedisCache : public ICacheService {
 public:
  static RedisCache& instance();
  void               incr(const std::string& key) override;
  void               remove(const std::string& key) override;
  bool               exists(const std::string& key) override;
  void               clearCache() override;
  void               clearPrefix(const std::string& prefix) override;

  void setPipelines(const std::vector<std::string>&    keys,
                    const std::vector<std::string>& results,
                    std::chrono::minutes               ttl = std::chrono::minutes(30)) override;

  void set(const std::string&        key,
           const std::string&     value,
           std::chrono::milliseconds ttl = std::chrono::minutes(30)) override;

  std::optional<std::string> get(const std::string& key) override;

 private:
  std::unique_ptr<sw::redis::Redis> redis_;
  std::mutex                        init_mutex_;

  // template <typename T>
  // std::string buildEntityKey(const T& entity, std::string table_name) const;

  // template <typename T>
  // long long getEntityId(const T& entity) const;

  sw::redis::Redis& getRedis();

  int getTtlWithJitter(std::chrono::seconds ttl);

  RedisCache()                             = default;
  RedisCache(const RedisCache&)            = delete;
  RedisCache& operator=(const RedisCache&) = delete;
};

#endif  // BACKEND_REDISCACHE_REDISCACHE_H_
