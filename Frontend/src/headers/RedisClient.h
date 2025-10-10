#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include "headers/ICash.h"
#include <sw/redis++/redis++.h>

class RedisClient : public ICash
{
    sw::redis::Redis redis;
public:
    RedisClient(std::string url) : redis(url) {}

    OptionalString get(const std::string& token) override{
        OptionalString takenOpt;
        try {
            takenOpt = redis.get("TOKEN");
        } catch (const sw::redis::Error &e) {
            qDebug() << "Redis error:" << e.what();
        }

        return takenOpt;
    }

    void saveToken(const std::string& tokenName, const std::string& token) override{
        redis.set(tokenName, token);
    }

    void deleteToken(const std::string& tokenName) override{
        redis.del("TOKEN");
    }
};

#endif // REDISCLIENT_H
