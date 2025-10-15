#ifndef REDISCACHE_H
#define REDISCACHE_H

#include <sw/redis++/redis++.h>
#include <nlohmann/json.hpp>
#include <QDebug>
using namespace sw::redis;

using namespace sw::redis;
using json = nlohmann::json;

class RedisCache {
public:
    static RedisCache& instance() {
        static RedisCache inst;
        return inst;
    }

    template<typename T, typename Duration = std::chrono::hours>
    void set(const std::string& key, const T& value, Duration ttl = std::chrono::hours(24)) {
        try {
            const std::string serialized = serialize(value);
            const auto ttlSeconds = getTtlWithJitter(ttl);

            redis->set(key, serialized);
            redis->expire(key, std::chrono::seconds(ttlSeconds));
        } catch (const std::exception& e) {
            logError("set", key, e);
        }
    }

    template<typename T>
    std::optional<T> get(const std::string& key) {
        try {
            auto val = redis->get(key);
            if (val) return deserialize<T>(*val);
        } catch (const std::exception& e) {
            logError("get", key, e);
        }
        return std::nullopt;
    }

    void remove(const std::string& key) {
        try {
            redis->del(key);
        } catch (const std::exception& e) {
            logError("remove", key, e);
        }
    }

    void clearPrefix(const std::string& prefix) {
        if (!redis) return;
        std::vector<std::string> keys;
        redis->keys(prefix + "*", std::back_inserter(keys));
        for (const auto& key : keys)
            redis->del(key);
    }

    bool exists(const std::string& key) {
        if (!redis) return false;
        return redis->exists(key);
    }

private:
    std::unique_ptr<Redis> redis;
    std::mutex initMutex;

    RedisCache() = default;

    Redis& getRedis() {
        if (!redis) {
            std::scoped_lock lock(initMutex);
            if (!redis) {
                try {
                    redis = std::make_unique<Redis>("tcp://127.0.0.1:6379");
                } catch (const std::exception& e) {
                    throw std::runtime_error(std::string("Redis init failed: ") + e.what());
                }
            }
        }
        return *redis;
    }


    template<typename T>
    std::string serialize(const T& value) {
        if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else {
            return json(value).dump();
        }
    }

    template<typename T>
    T deserialize(const std::string& str) {
        if constexpr (std::is_arithmetic_v<T>) {
            T val{};
            std::istringstream(str) >> val;
            return val;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return str;
        } else {
            return json::parse(str).get<T>();
        }
    }

    template<typename Duration>
    int getTtlWithJitter(Duration ttl) {
        const int baseSeconds = std::chrono::duration_cast<std::chrono::seconds>(ttl).count();
        const int jitter = (std::rand() % 61 - 30) * 60; // ±30 хвилин
        return std::max(baseSeconds + jitter, 300);
    }

    void logError(const std::string& action, const std::string& key, const std::exception& e) {
        qDebug() << "[RedisCache ERROR] " << action << "('" << key << "'): " << e.what();
    }

    RedisCache(const RedisCache&) = delete;
    RedisCache& operator=(const RedisCache&) = delete;
};

#endif // REDISCACHE_H
