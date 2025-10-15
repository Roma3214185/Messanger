#ifndef USER_H
#define USER_H

#include <string>
#include <DataBase/database.h>
#include <nlohmann/json.hpp>
#include "../../GenericRepository/GenericReposiroty.h"
#include "../../RedisCashe/RedisCache.h"

struct User
{
    long long id;
    std::string username;
    std::string email;
    std::string tag;
};

template<>
struct Reflection<User> {
    static Meta meta() {
        return {
            .name = "User",
            .tableName = "users",
            .fields = {
                make_field("id", &User::id),
                make_field("username", &User::username),
                make_field("email", &User::email)
            }
        };
    }
};


#endif // USER_H
