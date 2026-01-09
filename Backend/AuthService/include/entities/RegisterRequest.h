#ifndef BACKEND_AUTHSERVICE_SRC_HEADERS_REGISTERREQUEST_H_
#define BACKEND_AUTHSERVICE_SRC_HEADERS_REGISTERREQUEST_H_

#include <string>

struct RegisterRequest {
  std::string email;
  std::string password;
  std::string name;
  std::string tag;
};

struct LoginRequest {
  std::string email;
  std::string password;
};

#endif  // BACKEND_AUTHSERVICE_SRC_HEADERS_REGISTERREQUEST_H_
