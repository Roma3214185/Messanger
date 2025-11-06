#ifndef BACKEND_AUTHSERVICE_SRC_HEADERS_AUTHRESPONCE_H_
#define BACKEND_AUTHSERVICE_SRC_HEADERS_AUTHRESPONCE_H_

#include <optional>
#include <string>

#include "User.h"

struct AuthResponce {
  std::string token;
  std::optional<User> user;
};

#endif  // BACKEND_AUTHSERVICE_SRC_HEADERS_AUTHRESPONCE_H_
