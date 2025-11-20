#ifndef BACKEND_AUTHSERVICE_SRC_AUTHMANAGER_AUTHMANAGER_H_
#define BACKEND_AUTHSERVICE_SRC_AUTHMANAGER_AUTHMANAGER_H_

#include <optional>
#include <string>

#include "GenericRepository.h"
#include "entities/AuthResponce.h"
#include "authservice/interfaces/IAuthManager.h"

class RegisterRequest;
class LoginRequest;
class AuthResponce;

using OptionalUser     = std::optional<User>;

class AuthManager : public IAuthManager {
 public:
  AuthManager(GenericRepository& repository);
  OptionalUser  getUser(int user_id) override;
  OptionalUser  loginUser(const LoginRequest& login_request) override;
  OptionalUser  registerUser(const RegisterRequest& req) override;
  std::vector<User> findUserByTag(const std::string& tag) override;

 private:
  GenericRepository rep;
};

#endif  // BACKEND_AUTHSERVICE_SRC_AUTHMANAGER_AUTHMANAGER_H_
