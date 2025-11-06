#ifndef BACKEND_AUTHSERVICE_SRC_AUTHMANAGER_AUTHMANAGER_H_
#define BACKEND_AUTHSERVICE_SRC_AUTHMANAGER_AUTHMANAGER_H_

#include <optional>
#include <string>

#include "entities/AuthResponce.h"
#include "GenericRepository.h"

class RegisterRequest;
class LoginRequest;
class AuthResponce;

using OptionalResponce = std::optional<AuthResponce>;
using OptionalUser = std::optional<User>;

class AuthManager {
 public:
  AuthManager(GenericRepository& repository);
  OptionalResponce getUser(const std::string& token);
  OptionalResponce loginUser(const LoginRequest& login_request);
  OptionalResponce registerUser(const RegisterRequest& req);
  std::vector<User> findUserByTag(const std::string& tag);
  OptionalUser findUserById(int user_id);

 private:
  GenericRepository rep;
};

#endif  // BACKEND_AUTHSERVICE_SRC_AUTHMANAGER_AUTHMANAGER_H_
