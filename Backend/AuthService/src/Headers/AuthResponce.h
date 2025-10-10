#ifndef AUTHRESPONCE_H
#define AUTHRESPONCE_H

#include "Headers/User.h"

struct AuthResponce{
    std::string token;
    std::optional<User> user;
};

#endif // AUTHRESPONCE_H
