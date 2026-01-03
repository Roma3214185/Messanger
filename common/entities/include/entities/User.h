#ifndef BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_
#define BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

#include "interfaces/entity.h"

struct User final : public IEntity {
  long long id = 0;
  std::string username;
  std::string email;
  std::string tag;
  std::string avatar = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
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
        {"id", user.id}, {"email", user.email}, {"tag", user.tag}, {"username", user.username}};
  }
};

}  // namespace nlohmann

#endif  // BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_
