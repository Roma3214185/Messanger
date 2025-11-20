#include "authservice/authmanager.h"

#include <optional>
#include <string>

#include "authservice/JwtUtils.h"
#include "authservice/PasswordService.h"
#include "entities/RegisterRequest.h"
#include "Debug_profiling.h"
#include "entities/UserCredentials.h"

using std::nullopt;
using std::string;

OptionalUser AuthManager::getUser(int user_id) {
  return rep.findOne<User>(user_id);
}

OptionalUser AuthManager::loginUser(const LoginRequest& login_request) {
  auto finded_users = rep.findByField<User>(UserTable::Email, login_request.email);

  if (finded_users.empty()) {
    LOG_ERROR("User not found with email '{}'", login_request.email);
    return std::nullopt;
  }

  auto findedUser = finded_users.front();
  LOG_INFO("User found with email '{}', id is '{}'", login_request.email, findedUser.id);

  auto user_credentials_vector = rep.findByField<UserCredentials>("user_id", findedUser.id);
  assert(user_credentials_vector.size() == 1);
  auto user_credentials = user_credentials_vector.front();
  if (!PasswordService::verify(login_request.password, user_credentials.hash_password)) {
    LOG_ERROR("Invalid password");
    return std::nullopt;
  }

  return findedUser;
}

OptionalUser AuthManager::registerUser(const RegisterRequest& req) {
  auto users_with_same_email = rep.findByField<User>(UserTable::Email, req.email);
  if(!users_with_same_email.empty()) return std::nullopt;

  auto users_with_same_tag = rep.findByField<User>(UserTable::Tag, req.tag);
  if(!users_with_same_tag.empty()) return std::nullopt;

  User user_to_save{.username = req.name, .tag = req.tag, .email = req.email};

  bool saved = rep.save(user_to_save);
  if (!saved) return std::nullopt;

  std::string     hashed_password = PasswordService::hash(req.password);
  UserCredentials user_credentials;
  user_credentials.user_id       = user_to_save.id;
  user_credentials.hash_password = hashed_password;

  bool saved_credentials = rep.save(user_credentials);
  if (!saved_credentials) {
    LOG_ERROR("Server error while saving credentials");
    return std::nullopt;
  }

  return user_to_save;
}

std::vector<User> AuthManager::findUserByTag(const string& tag) {
  return rep.findByField<User>(UserTable::Tag, QString::fromStdString(tag));
}

AuthManager::AuthManager(GenericRepository& repository) : rep(repository) {}
