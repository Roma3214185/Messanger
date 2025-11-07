#ifndef USERCREDENTIALS_H
#define USERCREDENTIALS_H

#include <string>
#include <nlohmann/json.hpp>

struct UserCredentials {
  long long user_id;
  std::string hash_password;
};

template <>
struct Reflection<UserCredentials> {
    static Meta meta() {
      return Meta{
          .name = "Credentials",
          .table_name = "credentials",
          .fields = {
                     make_field<UserCredentials, long long>("id", &UserCredentials::user_id),
                     make_field<UserCredentials, std::string>("hash_password", &UserCredentials::hash_password)}
      };
    }
};

template <>
struct Builder<UserCredentials> {
    static Chat build(QSqlQuery& query) {
      UserCredentials user_credentials;
      int idx = 0;

      auto assign = [&](auto& field) -> void {
        using TField = std::decay_t<decltype(field)>;
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

inline constexpr auto UserCredentials =
    std::make_tuple(&UserCredentials::user_id, &UserCredentials::hash_password);

template <>
struct EntityFields<UserCredentials> {
    static constexpr auto& fields = UserCredentials;
};

//namespace nlohmann {

//template <>
//struct adl_serializer<UserCredentials> {
  static void to_json(nlohmann::json& json, const UserCredentials& user_credentials) {
    json = nlohmann::json{{"id", user_credentials.user_id},
                                  {"hash_password", user_credentials.hash_password}};
  }

  static void from_json(const nlohmann::json& json, UserCredentials& user_credentials) {
    json.at("id").get_to(user_credentials.user_id);
    json.at("hash_password").get_to(user_credentials.hash_password);
  }
//};

//}  // nlohmann

#endif // USERCREDENTIALS_H
