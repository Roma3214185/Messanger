#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include <sw/redis++/redis++.h>
#include <QDebug>
#include "../../DebugProfiling/Debug_profiling.h"
#include "headers/ICache.h"

class RedisClient : public ICache
{
    sw::redis::Redis redis;

public:

    RedisClient(std::string url) : redis(url) {}

    OptionalToken get(const Key& key) override{
        OptionalToken takenOpt;
        try {
            takenOpt = redis.get(key);
        } catch (const sw::redis::Error &e) {
            spdlog::error("Redis error: '{}'", e.what());
        }

        return takenOpt;
    }

    void saveToken(const Key& key, const Token& token) override{
        redis.set(key, token);
    }

    void deleteToken(const Key& key) override{
        redis.del(key);
    }
};

#endif // REDISCLIENT_H
