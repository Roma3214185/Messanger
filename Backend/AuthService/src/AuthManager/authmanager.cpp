#include "authmanager.h"

#include <JwtUtils.h>

#include <optional>
#include <string>

#include "Debug_profiling.h"
#include "RegisterRequest.h"

using std::nullopt;
using std::string;

OptionalResponce AuthManager::getUser(const string& token) {
  PROFILE_SCOPE("[AuthManager::getUser");
  std::optional<int> user_id_ptr = JwtUtils::verifyTokenAndGetUserId(token);
  if (!user_id_ptr) {
    spdlog::error("[getUser] Server can't verify token (NULLPTR)");
    return nullopt;
  }
  int user_id = *user_id_ptr;

  if (user_id == 0) {
    spdlog::error("[getUser] Server can't verify token (id is zero)");
    return nullopt;
  }

  LOG_INFO("[getUser] verified id = '{}'", user_id);

  auto finded_user = rep.findOne<User>(user_id);
  if (!finded_user) {
    spdlog::error("User with id not founded; id = '{}'", user_id);
    return nullopt;
  }

  LOG_INFO("User was founded; name = '{}' and id {}", finded_user->username, finded_user->id);
  return AuthResponce{.token = token, .user = finded_user};
}

OptionalResponce AuthManager::loginUser(const LoginRequest& login_request) {
  PROFILE_SCOPE("[AuthManager::loginUser");
  spdlog::debug("Try login user, email = '{}' and 'password '{}'",
                login_request.email, login_request.password);
  auto finded_users = rep.findByField<User>(
      "email", QString::fromStdString(login_request.email));

  // find hashedPassword and check
  if (finded_users.empty()) {
    spdlog::warn("User not found with email '{}'", login_request.email);
    return nullopt;
  }

  auto findedUser = finded_users.front();
  LOG_INFO("User found with email '{}', id is '{}'", login_request.email,
           findedUser.id);

  auto token = JwtUtils::generateToken(findedUser.id);
  return AuthResponce{.token = token, .user = findedUser};
}

OptionalResponce AuthManager::registerUser(const RegisterRequest& req) {
  PROFILE_SCOPE("AuthManager::registerUser");
  User user_to_save{
      .username = req.name,
      .tag = req.tag,
      .email = req.email  // u don't save password now
  };

  rep.save(user_to_save);
  auto token = JwtUtils::generateToken(user_to_save.id);

  return AuthResponce{.token = token, .user = user_to_save};
}

std::vector<User> AuthManager::findUserByTag(const string& tag) {
  PROFILE_SCOPE("AuthManager::findUserByTag");
  auto findedUsers = rep.findByField<User>("tag", QString::fromStdString(tag));
  // TODO(roma): make "tag" -> User::UserTag
  return findedUsers;
}

OptionalUser AuthManager::findUserById(int user_id) {
  PROFILE_SCOPE("AuthManager::findUserById");
  return rep.findOne<User>(user_id);
}

AuthManager::AuthManager(GenericRepository& repository) : rep(repository) {}
