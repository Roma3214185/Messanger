#ifndef USERCREDENTIALS_H
#define USERCREDENTIALS_H

#include <nlohmann/json.hpp>
#include <string>

#include "Fields.h"

struct UserCredentials {
  long long   user_id;
  std::string hash_password;
};

template <>
struct Reflection<UserCredentials> {
  static Meta meta() {
    return Meta{
        .table_name = UserCredentialsTable::Table,
        .fields     = {make_field<UserCredentials, long long>(UserCredentialsTable::UserId, &UserCredentials::user_id),
                       make_field<UserCredentials, std::string>(UserCredentialsTable::HashPassword,
                                                            &UserCredentials::hash_password)}};
  }
};

template <>
struct Builder<UserCredentials> {
  static UserCredentials build(QSqlQuery& query) {
    UserCredentials user_credentials;
    int             idx = 0;

    auto assign = [&](auto& field) -> void {
      using TField         = std::decay_t<decltype(field)>;
      const QVariant value = query.value(idx++);
      if constexpr (std::is_same_v<TField, long long>) {
        field = value.toLongLong();
      } else if constexpr (std::is_same_v<TField, int>) {
        field = value.toInt();
      } else if constexpr (std::is_same_v<TField, std::string>) {
        field = value.toString().toStdString();
      } else if constexpr (std::is_same_v<TField, QString>) {
        field = value.toString();
      } else {
        field = value.value<TField>();
      }
    };

    assign(user_credentials.user_id);
    assign(user_credentials.hash_password);

    return user_credentials;
  }
};

template <>
struct EntityKey<UserCredentials> {
  static std::string get(const UserCredentials& entity) { return std::to_string(entity.user_id); }
};

inline constexpr auto UserCredentialsFields =
    std::make_tuple(&UserCredentials::user_id, &UserCredentials::hash_password);

template <>
struct EntityFields<UserCredentials> {
  static constexpr auto& fields = UserCredentialsFields;
};

namespace nlohmann {

template <>
struct adl_serializer<UserCredentials> {
  static void to_json(nlohmann::json& json, const UserCredentials& user_credentials) {
    json = nlohmann::json{{UserCredentialsTable::UserId, user_credentials.user_id},
                          {UserCredentialsTable::HashPassword, user_credentials.hash_password}};
  }

  static void from_json(const nlohmann::json& json, UserCredentials& user_credentials) {
    json.at(UserCredentialsTable::UserId).get_to(user_credentials.user_id);
    json.at(UserCredentialsTable::HashPassword).get_to(user_credentials.hash_password);
  }
};

}  // namespace nlohmann

#endif  // USERCREDENTIALS_H
