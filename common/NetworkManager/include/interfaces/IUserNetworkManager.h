#ifndef IUSERNETWORKMANAGER_H
#define IUSERNETWORKMANAGER_H

#include "INetworkManager.h"

class User;

class IUserNetworkManager : public INetworkManager {
  public:
    std::optional<User> getUserById(int otherUserId);
};

#endif // IUSERNETWORKMANAGER_H
