#ifndef USER_H
#define USER_H

#include <string>

struct User
{
    std::string name;
    std::string email;
    int id;
    std::optional<std::string> password;
    std::string tag;
};

#endif // USER_H
