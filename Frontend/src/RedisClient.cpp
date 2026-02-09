#include "RedisClient.h"

#include <sw/redis++/redis++.h>

#include "Debug_profiling.h"

struct RedisClient::Impl {
    sw::redis::Redis redis;

    explicit Impl(const std::string& url)
        : redis(url) {}
};

RedisClient::RedisClient(const std::string& url)
    : impl_(std::make_unique<Impl>(url)) {}

RedisClient::~RedisClient() = default;

OptionalToken RedisClient::get(const Key &key) {
  try {
    return impl_->redis.get(key);
  } catch (const sw::redis::Error &e) {
    LOG_ERROR("Redis error: '{}'", e.what());
    return std::nullopt;
  }
}

void RedisClient::saveToken(const Key &key, const TokenStd &token) { impl_->redis.set(key, token); }

void RedisClient::deleteToken(const Key &key) { impl_->redis.del(key); }
