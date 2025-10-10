#ifndef REGISTERREQUEST_H
#define REGISTERREQUEST_H
#include <string>

struct RegisterRequest{
    std::string email;
    std::string password;
    std::string name;
    std::string tag;
};

#endif // REGISTERREQUEST_H
