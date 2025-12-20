#ifndef IUSERNETWORKMANAGER_H
#define IUSERNETWORKMANAGER_H

#include "INetworkManagerBase.h"
#include "entities/User.h"

class IUserNetworkManager : public virtual INetworkManagerBase {
  public:
    virtual std::optional<User> getUserById(long long otherUserId);
};

#endif // IUSERNETWORKMANAGER_H
