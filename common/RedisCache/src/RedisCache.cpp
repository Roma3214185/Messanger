#include "RedisCache.h"

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

void RedisCache::logError(const std::string&    action,
                          const std::string&    key,
                          const std::exception& e) {
  LOG_ERROR("'{}' ( '{}' ): '{}'", action, key, e.what());
}

void RedisCache::set(const std::string&        key,
                     const nlohmann::json&     value,
                     std::chrono::milliseconds ttl) {
  try {
    const std::string serialized_enity = value.dump();
    LOG_INFO("set for key '{}' value: '{}'", key, serialized_enity);
    auto ranged_ttl = getRangedTtl(ttl);
    getRedis().set(key, serialized_enity, ranged_ttl);
  } catch (const std::exception& e) {
    logError("set", key, e);
  }
}

std::optional<nlohmann::json> RedisCache::get(const std::string& key) {
  LOG_INFO("real get");
  try {
    if (auto value = getRedis().get(key)) return nlohmann::json::parse(*value);
  } catch (const std::exception& e) {
    logError("get", key, e);
  }
  return std::nullopt;
}

void RedisCache::setPipelines(const std::vector<std::string>&    keys,
                              const std::vector<nlohmann::json>& results,
                              std::chrono::minutes               ttl) {
  try {
    assert(keys.size() == results.size() || keys.size() == 1);

    auto pipe = redis_->pipeline();

    for (int i = 0; i < keys.size(); i++) {
      const std::string&    key   = keys.size() == 1 ? keys[0] : keys[i];
      const nlohmann::json& value = results[i];
      LOG_INFO("Save ({}), {}", key, value.dump());
      pipe.set(key, value.dump(), ttl);
    }

    pipe.exec();
    LOG_INFO("Saved {} entities in Redis pipeline", results.size());
  } catch (const sw::redis::Error& e) {
    LOG_ERROR("Redis pipeline failed: {}", e.what());
  }
}

void RedisCache::clearCache() {
  redis_->flushall();
}
