#ifndef AUTHRESPONCE_H
#define AUTHRESPONCE_H

#include <string>
#include <optional>

#include "User.h"

struct AuthResponce{
    std::string token;
    std::optional<User> user;
};

#endif // AUTHRESPONCE_H
