#ifndef ICHATNETWORKMANAGER_H
#define ICHATNETWORKMANAGER_H

#include "INetworkManagerBase.h"
#include <vector>

using UserId = long long;

class IChatNetworkManager : public virtual INetworkManagerBase {
  public:
    virtual std::vector<UserId> getMembersOfChat(long long chat_id);
};

#endif // ICHATNETWORKMANAGER_H
