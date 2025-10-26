#ifndef USER_H
#define USER_H

#include <string>

#include <nlohmann/json.hpp>

#include "GenericReposiroty.h"
#include "RedisCache.h"

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

inline constexpr auto UserFields = std::make_tuple(
    &User::id,
    &User::email,
    &User::tag,
    &User::username
    );

template<>
struct EntityFields<User> {
    static constexpr auto& fields = UserFields;
};

template<>
struct Builder<User> {
    static User build(QSqlQuery& query) {
        User e;
        int i = 0;

        auto assign = [&](auto& field) {
            using TField = std::decay_t<decltype(field)>;
            QVariant value = query.value(i++);
            if constexpr (std::is_same_v<TField, long long>)
                field = value.toLongLong();
            else if constexpr (std::is_same_v<TField, int>)
                field = value.toInt();
            else if constexpr (std::is_same_v<TField, std::string>)
                field = value.toString().toStdString();
            else if constexpr (std::is_same_v<TField, QString>)
                field = value.toString();
            else
                field = value.value<TField>();
        };

        return e;
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
