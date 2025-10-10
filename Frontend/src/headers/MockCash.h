#ifndef MOCKCASH_H
#define MOCKCASH_H

#include "ICash.h"
#include <unordered_map>

class MockCash : public ICash{
    std::unordered_map<std::string, std::string> cash;
public:
    std::optional<std::string> get(std::string tokenName) override{
        auto it = cash.find(tokenName);
        if(it != cash.end()) return cash[tokenName];
        return std::nullopt;
    }

    void saveToken(std::string tokenName, std::string token) override{
        cash[tokenName] = token;
    }

    void deleteToken(std::string tokenName) override{
        cash.erase(tokenName);
    }
};
#endif // MOCKCASH_H
