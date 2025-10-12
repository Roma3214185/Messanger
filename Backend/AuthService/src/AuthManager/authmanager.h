#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <string>
#include <optional>

#include "Headers/AuthResponce.h"
#include "Headers/RegisterRequest.h"
#include "DataBase/database.h"

using OptionalResponce = std::optional<AuthResponce>;
using OptionalUser = std::optional<User>;

class AuthManager
{
public:

    AuthManager(DataBase& repository);

    OptionalResponce getUser(const std::string& token);
    OptionalResponce loginUser(const std::string& email, const std::string& password);
    OptionalResponce registerUser(const RegisterRequest& req);
    QList<User> findUserByTag(const std::string& tag);
    OptionalUser findUserById(const int& userId);

private:

    DataBase& db;
};

#endif // AUTHSERVICE_H
