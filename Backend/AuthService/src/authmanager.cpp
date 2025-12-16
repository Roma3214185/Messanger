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
  return rep_.findOne<User>(user_id);
}

// OptionalUser AuthManager::findUserByEmail(const std::string& email) {
//   auto finded_users = rep_.findByField<User>(UserTable::Email, email);
//   return finded_users.empty() ? std::nullopt : std::make_optional(finded_users.front());
// }

OptionalUser AuthManager::findUserByEmail(const std::string& email) {
  auto query = QueryFactory::createSelect<User>(rep_.getExecutor(), rep_.getCache()); //TODO: remove executor and cache;
  query->from(UserTable::TableByEmail).where(UserTable::Email, email);
  auto finded_users = query->execute();
  return finded_users.empty() ? std::nullopt : std::make_optional(finded_users.front());
}

std::optional<UserCredentials> AuthManager::findUserCredentials(int user_id) {
  auto result = rep_.findByField<UserCredentials>(UserCredentialsTable::UserId, user_id);
  return result.empty() ? std::nullopt :std::make_optional(result.front());
}

OptionalUser AuthManager::loginUser(const LoginRequest& login_request) {
  auto user_with_same_email = findUserByEmail(login_request.email);

  if (!user_with_same_email) {
    LOG_ERROR("User not found with email '{}'", login_request.email);
    return std::nullopt;
  }

  LOG_INFO("User found with email '{}', id is '{}'", login_request.email, user_with_same_email->id);

  auto user_credentials = findUserCredentials(user_with_same_email->id);
  if (!user_credentials) {
    LOG_ERROR("User credentilas not found for user_id {} ", user_with_same_email->id);
    return std::nullopt;
  }

  if (!passwordIsValid(login_request.password, user_credentials->hash_password)) {
    LOG_ERROR("Invalid password: {}", login_request.password);
    return std::nullopt;
  }

  return user_with_same_email;
}

bool AuthManager::passwordIsValid(const std::string& password_to_check, const std::string& hash_password) {
  //TODO: make check if password_to_check satisfy condition (length, symbols, etc)
  return PasswordService::verify(password_to_check, hash_password);
}

OptionalUser AuthManager::registerUser(const RegisterRequest& req) {
  // auto user_with_same_email = findUserByEmail(req.email);
  // if(user_with_same_email) {
  //   LOG_WARN("There is user with email {} already, his email {} name is {} and tag is {}", req.email,
  //            user_with_same_email->email, user_with_same_email->username, user_with_same_email->tag);
  //   return std::nullopt;
  // }

  // auto users_with_same_tag = findUserWithSameTag(req.tag);
  // if(users_with_same_tag) {
  //   LOG_WARN("There is user with tag {} already, his email {} name is {} and tag is {}", req.tag,
  //            users_with_same_tag->email, users_with_same_tag->username, users_with_same_tag->tag);
  //   return std::nullopt;
  // }

  User user_to_save{.username = req.name, .email = req.email, .tag = req.tag};

  LOG_INFO("User_to_save: ", nlohmann::json(user_to_save).dump());
  if (!rep_.save(user_to_save)) {
    LOG_ERROR("Error while saving");
    return std::nullopt;
  }

  UserCredentials user_credentials;
  user_credentials.user_id       = user_to_save.id;
  user_credentials.hash_password = getHashPassword(req.password);

  LOG_INFO("User saved, try to save credentials {}", nlohmann::json(user_credentials).dump());
  bool saved_credentials = rep_.save(user_credentials); //TODO: make transaction -> save(vector<IEntity>){user_to_save, user_credentials}
  if (!saved_credentials) {
    LOG_ERROR("Server error while saving credentials");
    return std::nullopt;
  }

  LOG_INFO("Credentials are saved");
  return user_to_save;
}

std::string AuthManager::getHashPassword(const std::string& raw_passport) {
  return PasswordService::hash(raw_passport);
}

std::vector<User> AuthManager::findUsersByTag(const string& tag) {  //TODO: make trie
  return rep_.findByField<User>(UserTable::Tag, QString::fromStdString(tag));
}

std::optional<User> AuthManager::findUserWithSameTag(const std::string& tag) {
  auto result = rep_.findByField<User>(UserTable::Tag, QString::fromStdString(tag));
  return result.empty() ? std::nullopt : std::make_optional(result.front());
}

AuthManager::AuthManager(GenericRepository& repository) : rep_(repository) {}
