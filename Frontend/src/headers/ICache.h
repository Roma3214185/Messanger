#ifndef ICACHE_H
#define ICACHE_H

#include <string>

using Key = std::string;
using Token = std::string;
using OptionalToken = std::optional<Token>;

class ICache
{
public:
    virtual ~ICache() = default;

    virtual OptionalToken get(const Key& key) = 0;
    virtual void saveToken(const Key& key, const Token& token) = 0;
    virtual void deleteToken(const Key& key) = 0;
};

#endif // ICACHE_H
