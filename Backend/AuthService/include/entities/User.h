#ifndef BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_
#define BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

#include "Persistence/GenericRepository.h"
#include "RedisCache.h"

struct User {
  long long id;
  std::string username;
  std::string email;
  std::string tag;
};

template <>
struct Reflection<User> {
  static Meta meta() {
    return {
        .name = "User",
        .table_name = "users",
        .fields = {make_field<User, long long>("id", &User::id),
                   make_field<User, std::string>("username", &User::username),
                   make_field<User, std::string>("tag", &User::tag),
                   make_field<User, std::string>("email", &User::email)}};
  }
};

inline constexpr auto UserFields =
    std::make_tuple(&User::id, &User::email, &User::tag, &User::username);

template <>
struct EntityFields<User> {
  static constexpr auto& fields = UserFields;
};

template <>
struct Builder<User> {
  static User build(QSqlQuery& query) {
    User user;
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

    assign(user.id);
    assign(user.username);
    assign(user.email);
    assign(user.tag);

    return user;
  }
};


namespace nlohmann {

template <>
struct adl_serializer<User> {
    static void from_json(const json& j, User& user) {
      j.at("id").get_to(user.id);
      j.at("email").get_to(user.email);
      j.at("tag").get_to(user.tag);
      j.at("username").get_to(user.username);
    }

    static void to_json(json& j, const User& user) {
      j = json{
          {"id", user.id},
          {"email", user.email},
          {"tag", user.tag},
          {"username", user.username}
      };
    }
};

} // namespace nlohmann

#endif  // BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_
