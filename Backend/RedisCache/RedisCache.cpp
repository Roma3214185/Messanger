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
