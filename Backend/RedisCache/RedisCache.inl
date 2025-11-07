#pragma once

template <typename T>
void RedisCache::saveEntities(const std::vector<T>& results, std::string table_name,
                  std::chrono::minutes ttl) {
  try {
    auto pipe = redis_->pipeline();

    for (const auto& entity : results) {
      std::string entityKey = buildEntityKey(entity, table_name);
      auto value = serialize(entity);
      redis_->set(entityKey, value, ttl);
    }

    pipe.exec();
    LOG_INFO("Saved {} entities in Redis pipeline", results.size());

  } catch (const sw::redis::Error& e) {
    LOG_ERROR("Redis pipeline failed: {}", e.what());
  }
}

template <typename T>
void RedisCache::saveEntity(const T& entity, std::string table_name,
                std::chrono::minutes ttl) {
  std::string entity_key = buildEntityKey(entity, table_name);
  auto value = serialize(entity);
  set(entity_key, value, ttl);
}

template <typename T, typename Duration>
void RedisCache::set(const std::string& key, const T& value,
         Duration ttl) {
  try {
    const std::string serialized_enity = serialize(value);
    LOG_INFO("set for key '{}' value: '{}'", key, serialized_enity);
    const auto ttlSeconds = getTtlWithJitter(ttl);

    getRedis().set(key, serialized_enity);
    getRedis().expire(key, std::chrono::seconds(ttlSeconds));

  } catch (const std::exception& e) {
    logError("set", key, e);
  }
}

template <typename T>
std::optional<T> RedisCache::get(const std::string& key) {
  try {
    auto value = getRedis().get(key);
    if (value) {
      LOG_INFO("For key '{}' finded cashe", key);
      return deserialize<T>(*value);
    }
  } catch (const std::exception& e) {
    logError("get", key, e);
  }
  return std::nullopt;
}

template <typename T>
std::string RedisCache::buildEntityKey(const T& entity, std::string table_name) const {
  return "entity_cache:" + table_name + ":" +
         std::to_string(getEntityId(entity));
}

template <typename T>
long long RedisCache::getEntityId(const T& entity) const {
  return entity.id;
}

template <typename T>
std::string RedisCache::serialize(const T& value) {
  if constexpr (std::is_arithmetic_v<T>) {
    return std::to_string(value);
  } else if constexpr (std::is_same_v<T, std::string>) {
    return value;
  } else {
    return nlohmann::json(value).dump();
  }
}

template <typename T>
T RedisCache::deserialize(const std::string& str) {
  if constexpr (std::is_arithmetic_v<T>) {
    T val{};
    std::istringstream(str) >> val;
    return val;
  } else if constexpr (std::is_same_v<T, std::string>) {
    return str;
  } else {
    return nlohmann::json::parse(str).get<T>();
  }
}

template <typename Duration>
int RedisCache::getTtlWithJitter(Duration ttl) {
  const int baseSeconds =
      std::chrono::duration_cast<std::chrono::seconds>(ttl).count();
  const int jitter = (std::rand() % 61 - 30) * 60;  // +-30 minutes
  return std::max(baseSeconds + jitter, 300);
}

