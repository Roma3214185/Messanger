#include "RedisCache.h"

#include <random>

namespace {

template <typename Duration>
std::chrono::milliseconds getRangedTtl(Duration ttl) {
  using namespace std::chrono;

  auto baseMs = duration_cast<milliseconds>(ttl).count();

  static std::random_device          rd;
  static std::mt19937                gen(rd());
  std::uniform_int_distribution<int> dist(-30 * 60 * 1000, 30 * 60 * 1000);  // +-30 min in ms
  int                                jitterMs = dist(gen);

  return milliseconds(std::max(static_cast<int>(baseMs + jitterMs), 300));
}

}  // namespace

void RedisCache::clearPrefix(const std::string& prefix) {
  std::vector<std::string> keys;
  getRedis().keys(prefix + "*", std::back_inserter(keys));
  for (const auto& key : keys) {
    getRedis().del(key);
  }
}

bool RedisCache::exists(const std::string& key) {
  bool exist = getRedis().exists(key);
  return exist;
}

void RedisCache::remove(const std::string& key) {
  try {
    getRedis().del(key);
  } catch (const std::exception& e) {

  }
}

void RedisCache::incr(const std::string& key) {
  redis_->incr(key);
}

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
        throw std::runtime_error(std::string("Redis init failed: ") + e.what());
      }
    }
  }
  return *redis_;
}

void RedisCache::set(const std::string&        key,
                     const std::string&     value,
                     std::chrono::milliseconds ttl) {
  try {
    auto ranged_ttl = getRangedTtl(ttl);
    getRedis().set(key, value, ranged_ttl);
  } catch (const std::exception& e) {

  }
}

std::optional<std::string> RedisCache::get(const std::string& key) {
  try {
    if (auto value = getRedis().get(key)) return *value;
  } catch (const std::exception& e) {
    return std::nullopt;
  }
  return std::nullopt;
}

void RedisCache::setPipelines(const std::vector<std::string>&    keys,
                              const std::vector<std::string>& results,
                              std::chrono::minutes               ttl) {
  try {
    assert(keys.size() == results.size() || keys.size() == 1);

    auto pipe = redis_->pipeline();

    for (int i = 0; i < keys.size(); i++) {
      const std::string&    key   = keys.size() == 1 ? keys[0] : keys[i];
      const std::string& value = results[i];
      pipe.set(key, value, ttl);
    }

    pipe.exec();
  } catch (const sw::redis::Error& e) {

  }
}

void RedisCache::clearCache() {
  redis_->flushall();
}
