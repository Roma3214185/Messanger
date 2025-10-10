#ifndef ICASH_H
#define ICASH_H

#include <QString>

class ICash{
public:
    virtual std::optional<std::string> get(std::string token) = 0;
    virtual void saveToken(std::string tokenName, std::string token) = 0;
    virtual void deleteToken(std::string tokenName) = 0;
};

#endif // ICASH_H
