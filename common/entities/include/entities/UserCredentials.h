#ifndef USERCREDENTIALS_H
#define USERCREDENTIALS_H

#include <nlohmann/json.hpp>
#include <string>

#include "Fields.h"
#include "interfaces/entity.h"

struct UserCredentials final : public IEntity {
  long long   user_id{0};
  std::string hash_password;
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
