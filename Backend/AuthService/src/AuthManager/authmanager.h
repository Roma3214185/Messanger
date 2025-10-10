#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <string>
#include <optional>
#include "Headers/AuthResponce.h"
#include "Headers/RegisterRequest.h"
#include "UserRepository/userrepository.h"

class AuthManager
{
public:
    AuthManager(UserRepository& repository);
    std::optional<AuthResponce> getUser(std::string token);
    std::optional<AuthResponce> loginUser(std::string email, std::string password);
    std::optional<AuthResponce> registerUser(RegisterRequest req);
    QList<User> findUserByTag(std::string tag);
    std::optional<User> findUserById(int userId);

private:
    UserRepository rep;
};

#endif // AUTHSERVICE_H
