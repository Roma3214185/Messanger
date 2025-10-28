#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_USER_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_USER_H_

#include <string>

struct User {
  std::string name;
  std::string email;
  int id;
  std::optional<std::string> password;
  std::string tag;
  std::string avatar = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
};

#endif  // BACKEND_CHATSERVICE_SRC_HEADERS_USER_H_
