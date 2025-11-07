#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_USER_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_USER_H_

#include <string>
#include <nlohmann/json.hpp>

struct User {
  std::string username;
  std::string email;
  int id;
  std::optional<std::string> password;
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
        {"id", user.id},
        {"email", user.email},
        {"tag", user.tag},
        {"username", user.username}
    };
  }
};

} // namespace nlohmann


#endif  // BACKEND_CHATSERVICE_SRC_HEADERS_USER_H_
