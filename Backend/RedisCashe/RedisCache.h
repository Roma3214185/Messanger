#ifndef REDISCACHE_H
#define REDISCACHE_H

#include <sw/redis++/redis++.h>
#include <nlohmann/json.hpp>
//#include <QDebug>
#include <iostream>
#include "../../DebugProfiling/Debug_profiling.h"
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
            LOG_INFO("set for key '{}' value: '{}'", key, serialized);
            const auto ttlSeconds = getTtlWithJitter(ttl);

            getRedis().set(key, serialized);
            getRedis().expire(key, std::chrono::seconds(ttlSeconds));

        } catch (const std::exception& e) {
            logError("set", key, e);
        }
    }

    void incr(const std::string& key){
        LOG_INFO("INCREMENT key: '{}'", key);
        redis->incr(key);
    }

    template<typename T>
    std::optional<T> get(const std::string& key) {
        try {
            auto val = getRedis().get(key);
            if (val) {
                LOG_INFO("For key '{}' finded cashe", key);
                return deserialize<T>(*val);
            }
        } catch (const std::exception& e) {
            logError("get", key, e);
        }
        return std::nullopt;
    }

    void remove(const std::string& key) {
        try {
            LOG_INFO("Remove key '{}' from cashe", key);
            getRedis().del(key);
        } catch (const std::exception& e) {
            logError("remove", key, e);
        }
    }

    void clearPrefix(const std::string& prefix) {
        LOG_INFO("clearPrefix prefix '{}' from cashe", prefix);
        std::vector<std::string> keys;
        getRedis().keys(prefix + "*", std::back_inserter(keys));
        for (const auto& key : keys)
            getRedis().del(key);
    }

    bool exists(const std::string& key) {
        bool exist = getRedis().exists(key);
        LOG_INFO("For key '{}' value exist:", key, exist);
        return exist;
    }


private:
    std::unique_ptr<Redis> redis;
    std::mutex initMutex;

    RedisCache(){

    }

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
        const int jitter = (std::rand() % 61 - 30) * 60; //+-30 minutes
        return std::max(baseSeconds + jitter, 300);
    }

    void logError(const std::string& action, const std::string& key, const std::exception& e) {
        LOG_ERROR("'{}' ( '{}' ): '{}'", action, key, e.what());
    }

    RedisCache(const RedisCache&) = delete;
    RedisCache& operator=(const RedisCache&) = delete;
};

#endif // REDISCACHE_H
