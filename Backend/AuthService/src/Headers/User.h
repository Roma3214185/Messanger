#ifndef USER_H
#define USER_H

#include <string>
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
                make_field<User, long long>("id", &User::id),
                make_field<User, std::string>("username", &User::username),
                make_field<User, std::string>("tag", &User::tag),
                make_field<User, std::string>("email", &User::email)
            }
        };
    }
};

inline void to_json(json& j, const User& u) {
    j = json{
        {"id", u.id},
        {"email", u.email},
        {"tag", u.tag},
        {"username", u.username}
    };
}

inline void from_json(const json& j, User& u) {
    j.at("id").get_to(u.id);
    j.at("email").get_to(u.email);
    j.at("tag").get_to(u.tag);
    j.at("username").get_to(u.username);
}


#endif // USER_H
