#ifndef BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_
#define BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

#include "Debug_profiling.h"

struct User final {
  long long id = 0;
  std::string username;
  std::string email;
  std::string tag;
  std::string avatar;

  User() = default;

  User(long long id, std::string username, std::string email, std::string tag,
       std::string avatar = "/Users/roma/QtProjects/Chat/default_avatar.jpeg")
      : id(id), username(std::move(username)), email(std::move(email)), tag(std::move(tag)), avatar(std::move(avatar)) {
    DBC_REQUIRE(checkInvariants());
  }

  bool checkInvariants() const { return id > 0 && !username.empty() && !email.empty() && !tag.empty(); }
};

namespace nlohmann {

template <>
struct adl_serializer<User> {
  static void from_json(const json &j, User &user) {
    user = User{j.at("id").get<long long>(), j.at("username").get<std::string>(), j.at("email").get<std::string>(),
                j.at("tag").get<std::string>(), j.value("avatar", "/Users/roma/QtProjects/Chat/default_avatar.jpeg")};
  }

  static void to_json(json &j, const User &user) {
    j = json{{"id", user.id}, {"email", user.email}, {"tag", user.tag}, {"username", user.username}};
  }
};

}  // namespace nlohmann

#endif  // BACKEND_AUTHSERVICE_SRC_HEADERS_USER_H_
