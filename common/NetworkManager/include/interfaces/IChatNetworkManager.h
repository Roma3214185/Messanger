#ifndef ICHATNETWORKMANAGER_H
#define ICHATNETWORKMANAGER_H

#include "INetworkManager.h"
#include <QVector>

using UserId = int;

class IChatNetworkManager : public INetworkManager {
  public:
    QVector<UserId> getMembersOfChat(int chat_id);
};

#endif // ICHATNETWORKMANAGER_H
