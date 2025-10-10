#ifndef ICASH_H
#define ICASH_H

#include <QString>

using OptionalString = std::optional<std::string>;

class ICash{
public:
    virtual OptionalString get(const std::string& token) = 0;
    virtual void saveToken(const std::string& tokenName, const std::string& token) = 0;
    virtual void deleteToken(const std::string& tokenName) = 0;
    virtual ~ICash() = default;
};

#endif // ICASH_H
