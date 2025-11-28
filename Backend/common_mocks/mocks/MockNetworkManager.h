#ifndef MOCKNETWORKMANAGER_H
#define MOCKNETWORKMANAGER_H

#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"

class MockNetworkManager
    : public IChatNetworkManager
    , public IUserNetworkManager
    , public IMessageNetworkManager {
    std::unordered_map<int, std::vector<int>> mp;
  public:
    int call_getMembersOfChat = 0;
    int call_getUserById = 0;
    int last_chat_id;
    int last_user_id;
    std::optional<User> mock_user;

    void setChatMembers(int chat_id, std::vector<int> ids) {
      mp[chat_id] = ids;
    }

    std::optional<User> getUserById(int otherUserId) {
      ++call_getUserById;
      last_user_id = otherUserId;
      return mock_user;
    }

    std::vector<UserId> getMembersOfChat(int chat_id) override {
      ++call_getMembersOfChat;
      last_chat_id = chat_id;
      return mp[chat_id];
    }
};

#endif // MOCKNETWORKMANAGER_H
