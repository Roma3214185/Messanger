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
class UserCredentials;

using OptionalUser     = std::optional<User>;

class AuthManager : public IAuthManager {
 public:
  AuthManager(GenericRepository& repository);
  OptionalUser  getUser(int user_id) override;
  OptionalUser  loginUser(const LoginRequest& login_request) override;
  OptionalUser  registerUser(const RegisterRequest& req) override;
  std::vector<User> findUsersByTag(const std::string& tag) override;

 protected:
  virtual OptionalUser findUserByEmail(const std::string& email);
  virtual std::optional<UserCredentials> findUserCredentials(int user_id);
  virtual bool passwordIsValid(const std::string& password_to_check, const std::string& hash_password);
  virtual std::string getHashPassword(const std::string& raw_passport);
  virtual std::optional<User> findUserWithSameTag(const std::string& tag);

 private:
  GenericRepository& rep_;
};

#endif  // BACKEND_AUTHSERVICE_SRC_AUTHMANAGER_AUTHMANAGER_H_
