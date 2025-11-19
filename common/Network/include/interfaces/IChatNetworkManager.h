#ifndef ICHATNETWORKMANAGER_H
#define ICHATNETWORKMANAGER_H

#include "INetworkManagerBase.h"
#include <QVector>

using UserId = int;

class IChatNetworkManager : public virtual INetworkManagerBase {
  public:
    virtual QVector<UserId> getMembersOfChat(int chat_id);
};

#endif // ICHATNETWORKMANAGER_H
