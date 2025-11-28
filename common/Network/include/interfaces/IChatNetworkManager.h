#ifndef ICHATNETWORKMANAGER_H
#define ICHATNETWORKMANAGER_H

#include "INetworkManagerBase.h"
#include <vector>

using UserId = int;

class IChatNetworkManager : public virtual INetworkManagerBase {
  public:
    virtual std::vector<UserId> getMembersOfChat(int chat_id);
};

#endif // ICHATNETWORKMANAGER_H
