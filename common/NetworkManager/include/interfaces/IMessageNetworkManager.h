#ifndef IMESSAGENETWORKMANAGER_H
#define IMESSAGENETWORKMANAGER_H

#include "interfaces/INetworkManager.h"
#include <QVector>

using UserId = int;

class IMessageNetworkManager : public INetworkManager {
    QVector<UserId> getMembersOfChat(int chat_id);
};

#endif // IMESSAGENETWORKMANAGER_H
