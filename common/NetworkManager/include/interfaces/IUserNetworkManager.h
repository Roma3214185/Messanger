#ifndef IUSERNETWORKMANAGER_H
#define IUSERNETWORKMANAGER_H

#include "INetworkManagerBase.h"

class User;

class IUserNetworkManager : public virtual INetworkManagerBase {
  public:
    virtual std::optional<User> getUserById(int otherUserId);
};

#endif // IUSERNETWORKMANAGER_H
