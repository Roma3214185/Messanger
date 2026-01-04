#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include <sw/redis++/redis++.h>

#include "Debug_profiling.h"
#include "interfaces/ICache.h"

class RedisClient : public ICache {
 public:
  RedisClient(std::string url) : redis(url) {}

  OptionalToken get(const Key& key) override {
    try {
      return redis.get(key);
    } catch (const sw::redis::Error& e) {
      spdlog::error("Redis error: '{}'", e.what());
      return std::nullopt;
    }

  }

  void saveToken(const Key& key, const Token& token) override { redis.set(key, token); }

  void deleteToken(const Key& key) override { redis.del(key); }

 private:
  sw::redis::Redis redis;
};

#endif  // REDISCLIENT_H
