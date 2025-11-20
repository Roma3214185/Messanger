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
  // PROFILE_SCOPE("[AuthManager::getUser");
  // std::optional<int> user_id_ptr = JwtUtils::verifyTokenAndGetUserId(token);
  // if (!user_id_ptr || *user_id_ptr == 0) {
  //   LOG_ERROR("[getUser] Invalid or exprired token");
  //   return nullopt;
  // }
  // int user_id = *user_id_ptr;
  // LOG_INFO("[getUser] verified id = '{}'", user_id);

  auto finded_user = rep.findOne<User>(user_id);
  if (!finded_user) {
    LOG_ERROR("User with id not founded; id = '{}'", user_id);
    return std::nullopt;
  }

  LOG_INFO("User was founded; name = '{}' and id {}", finded_user->username, finded_user->id);
  return finded_user;
}

OptionalUser AuthManager::loginUser(const LoginRequest& login_request) {
  auto finded_users = rep.findByField<User>("email", QString::fromStdString(login_request.email));

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
  auto findedUsers = rep.findByField<User>("tag", QString::fromStdString(tag));
  // TODO(roma): make "tag" -> User::UserTag |
  return findedUsers;
}

AuthManager::AuthManager(GenericRepository& repository) : rep(repository) {}
