#ifndef IUSERNETWORKMANAGER_H
#define IUSERNETWORKMANAGER_H

#include <optional>

#include "INetworkManagerBase.h"
#include "entities/User.h"

class IUserNetworkManager : public virtual INetworkManagerBase {
public:
  virtual std::optional<User> getUserById(long long other_user_id);
};

#endif // IUSERNETWORKMANAGER_H
