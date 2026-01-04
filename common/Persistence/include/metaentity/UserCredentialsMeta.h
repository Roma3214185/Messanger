#ifndef METAENTITY_USERCREDENTIALS_H
#define METAENTITY_USERCREDENTIALS_H

#include "Meta.h"
#include "entities/UserCredentials.h"

template <>
struct Reflection<UserCredentials> {
  static Meta meta() {
    return Meta{
        .table_name = UserCredentialsTable::Table,
        .fields     = {make_field<UserCredentials, long long>(UserCredentialsTable::UserId,
                                                          &UserCredentials::user_id),
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

#endif  // USERCREDENTIALS_H
