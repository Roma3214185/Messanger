#ifndef IUSERNETWORKMANAGER_H
#define IUSERNETWORKMANAGER_H

#include <optional>
#include "entities/User.h"

class IUserNetworkManager {
 public:
  virtual std::optional<User> getUserById(long long other_user_id) = 0;
};

class ProxyClient;

class UserNetworkManager : public IUserNetworkManager {
 public:
  UserNetworkManager(ProxyClient* proxy);
  std::optional<User> getUserById(long long other_user_id) override;

 private:
  ProxyClient* proxy_;
};

#endif  // IUSERNETWORKMANAGER_H
