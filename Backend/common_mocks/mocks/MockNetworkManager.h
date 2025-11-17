#ifndef MOCKNETWORKMANAGER_H
#define MOCKNETWORKMANAGER_H

#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"

class MockNetworkManager
    : public IChatNetworkManager
    , public IUserNetworkManager
    , public IMessageNetworkManager {
    std::unordered_map<int, QVector<int>> mp;
  public:
    int call_getMembersOfChat = 0;
    int last_chat_id;

    void setChatMembers(int chat_id, QVector<int> ids) {
      mp[chat_id] = ids;
    }

    QVector<UserId> getMembersOfChat(int chat_id) override {
      ++call_getMembersOfChat;
      last_chat_id = chat_id;
      return mp[chat_id];
    }
};

#endif // MOCKNETWORKMANAGER_H
