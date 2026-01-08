#ifndef METAENTITY_USER_H
#define METAENTITY_USER_H

#include "Meta.h"
#include "entities/User.h"

template <>
struct Reflection<User> {
  static Meta meta() {
    return {.table_name = "users",
            .fields = {make_field<User, long long>("id", &User::id),
                       make_field<User, std::string>("username", &User::username),
                       make_field<User, std::string>("tag", &User::tag),
                       make_field<User, std::string>("email", &User::email)}};
  }
};

inline constexpr auto UserFields = std::make_tuple(&User::id, &User::email, &User::tag, &User::username);

template <>
struct EntityFields<User> {
  static constexpr auto &fields = UserFields;
};

template <>
struct Builder<User> {
  static User build(QSqlQuery &query) {
    User user;
    int i = 0;

    auto assign = [&](auto &field) {
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

#endif  // METAENTITY_USER_H
