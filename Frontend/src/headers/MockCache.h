#ifndef MOCKCACHE_H
#define MOCKCACHE_H

#include "ICache.h"
#include <unordered_map>

using CacheMap = std::unordered_map<Key, Token>;

class MockCache : public ICache
{
    CacheMap cache;

public:

    OptionalToken get(const Key& key) override{
        auto it = cache.find(key);
        if(it != cache.end()) return it->second;
        return std::nullopt;
    }

    void saveToken(const Key& key, const Token& token) override{
        cache[key] = token;
    }

    void deleteToken(const Key& key) override{
        cache.erase(key);
    }
};

#endif // MOCKCACHE_H
