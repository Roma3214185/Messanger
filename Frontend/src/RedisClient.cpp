#include "RedisClient.h"

RedisClient::RedisClient(std::string url) : redis(std::move(url)) {}

OptionalToken RedisClient::get(const Key &key) {
  try {
    return redis.get(key);
  } catch (const sw::redis::Error &e) {
    spdlog::error("Redis error: '{}'", e.what());
    return std::nullopt;
  }
}

void RedisClient::saveToken(const Key &key, const TokenStd &token) { redis.set(key, token); }

void RedisClient::deleteToken(const Key &key) { redis.del(key); }
