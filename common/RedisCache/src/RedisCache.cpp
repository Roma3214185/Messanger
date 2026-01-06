#include "RedisCache.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <iterator>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

#include "Debug_profiling.h"
#include "sw/redis++/errors.h"
#include "sw/redis++/queued_redis.h"
//#include "sw/redis++/queued_redis.hpp"
//#include "sw/redis++/redis.hpp"

namespace {

template <typename Duration> std::chrono::seconds getRangedTtl(Duration ttl) {
  using namespace std::chrono;

  const auto base_ms = duration_cast<seconds>(ttl).count();
  constexpr int kMinTimeOfTtl = 300;

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(-30 * 60 * 1000,
                                          30 * 60 * 1000); // +-30 min in ms
  const int jitter_ms = dist(gen);

  return seconds(
      std::max(static_cast<int>(base_ms + jitter_ms), kMinTimeOfTtl));
}

} // namespace

void RedisCache::clearPrefix(const std::string &prefix) {
  std::vector<std::string> keys;
  getRedis().keys(prefix + "*", std::back_inserter(keys));
  for (const auto &key : keys) {
    getRedis().del(key);
  }
}

void RedisCache::remove(const std::string &key) {
  try {
    getRedis().del(key);
  } catch (const std::exception &e) {
    LOG_ERROR("Error to delete key: {}", key);
  }
}

void RedisCache::incr(const std::string &key) { redis_->incr(key); }

RedisCache &RedisCache::instance() {
  static RedisCache inst;
  return inst;
}

sw::redis::Redis &RedisCache::getRedis() {
  if (!redis_) {
    const std::scoped_lock lock(init_mutex_);
    try {
      redis_ = std::make_unique<sw::redis::Redis>("tcp://127.0.0.1:6379");
    } catch (const std::exception &e) {
      throw std::runtime_error(std::string("Redis init failed: ") + e.what());
    }
  }
  return *redis_;
}

void RedisCache::set(const std::string &key, const std::string &value,
                     std::chrono::seconds ttl) {
  try {
    auto ranged_ttl = getRangedTtl(ttl);
    getRedis().set(key, value, ranged_ttl);
    LOG_INFO("Set {} - {} for {} seconds", key, value, ranged_ttl.count());
  } catch (const std::exception &e) {
    LOG_ERROR("Error whyle set {} and value {} - error", key, value, e.what());
  } catch (...) {
    LOG_ERROR("Invalid Error whyle set {} and value {}", key, value);
  }
}

std::optional<std::string> RedisCache::get(const std::string &key) {
  try {
    // if (auto value = getRedis().get(key)) return *value;
    return getRedis().get(key);
  } catch (const std::exception &e) {
    LOG_ERROR("Error whyle get {} and value {} - error", key, e.what());
    return std::nullopt;
  } catch (...) {
    LOG_ERROR("Invalid Error whyle get {} and value", key);
    return std::nullopt;
  }

  return std::nullopt;
}

void RedisCache::setPipelines(const std::vector<std::string> &keys,
                              const std::vector<std::string> &results,
                              std::chrono::seconds ttl) {
  try {
    assert(keys.size() == results.size() || keys.size() == 1);

    auto pipe = redis_->pipeline();

    for (size_t i = 0; i < keys.size(); ++i) {
      const std::string &key = keys.size() == 1 ? keys[0] : keys[i];
      const std::string &value = results[i];
      pipe.set(key, value, ttl);
    }

    pipe.exec();
  } catch (const sw::redis::Error &e) {
    LOG_ERROR("Error set pipline {}", e.what());
  } catch (...) {
    LOG_ERROR("Invalid Error set pipline {}");
  }
}

void RedisCache::clearCache() { redis_->flushall(); }
