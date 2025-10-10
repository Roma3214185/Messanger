#ifndef USER_H
#define USER_H
#include <string>

struct User{
    std::string name;
    std::string email;
    int id;
    std::optional<std::string> password;
    std::string tag;
    std::string avatar = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
};

#endif // USER_H
