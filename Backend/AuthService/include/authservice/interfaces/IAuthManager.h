#ifndef IAUTHMANAGER_H
#define IAUTHMANAGER_H

#include <optional>

#include "entities/AuthResponce.h"
#include "entities/RegisterRequest.h"
#include "entities/User.h"

class IAuthManager {
 public:
  using OptionalResponce = std::optional<AuthResponce>;
  using OptionalUser = std::optional<User>;

  virtual OptionalUser getUser(long long user_id) = 0;
  virtual OptionalUser loginUser(const LoginRequest &login_request) = 0;
  virtual OptionalUser registerUser(const RegisterRequest &req) = 0;
  virtual std::vector<User> findUsersByTag(const std::string &tag) = 0;
  virtual ~IAuthManager() = default;
};

#endif  // IAUTHMANAGER_H
