#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include "headers/ICash.h"
#include <sw/redis++/redis++.h>

class RedisClient : public ICash
{
    sw::redis::Redis redis;
public:
    RedisClient(std::string url) : redis(url) {}

    std::optional<std::string> get(std::string token) override{
        std::optional<std::string> takenOpt;
        try {
            takenOpt = redis.get("TOKEN");
        } catch (const sw::redis::Error &e) {
            qDebug() << "Redis error:" << e.what();
        }

        return takenOpt;
    }

    void saveToken(std::string tokenName, std::string token) override{
        redis.set(tokenName, token);
    }

    void deleteToken(std::string tokenName) override{
        redis.del("TOKEN");
    }
};

#endif // REDISCLIENT_H
