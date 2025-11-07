#include "RedisCache.h"

void RedisCache::clearPrefix(const std::string& prefix) {
  LOG_INFO("clearPrefix prefix '{}' from cashe", prefix);
  std::vector<std::string> keys;
  getRedis().keys(prefix + "*", std::back_inserter(keys));
  for (const auto& key : keys) {
      getRedis().del(key);
  }
}

bool RedisCache::exists(const std::string& key) {
  bool exist = getRedis().exists(key);
  LOG_INFO("For key '{}' value exist:", key, exist);
  return exist;
}

void RedisCache::remove(const std::string& key) {
  try {
    LOG_INFO("Remove key '{}' from cashe", key);
    getRedis().del(key);
  } catch (const std::exception& e) {
    logError("remove", key, e);
  }
}

void RedisCache::incr(const std::string& key) {
  LOG_INFO("INCREMENT key: '{}'", key);
  redis_->incr(key);
}

void RedisCache::clearCache() { redis_->flushdb(); }

RedisCache& RedisCache::instance() {
  static RedisCache inst;
  return inst;
}

sw::redis::Redis& RedisCache::getRedis() {
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

void RedisCache::logError(const std::string& action, const std::string& key,
              const std::exception& e) {
  LOG_ERROR("'{}' ( '{}' ): '{}'", action, key, e.what());
}
