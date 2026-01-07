#ifndef MOCKNETWORKMANAGER_H
#define MOCKNETWORKMANAGER_H

#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"

class MockNetworkManager : public IChatNetworkManager,
                           public IUserNetworkManager,
                           public IMessageNetworkManager {
  std::unordered_map<long long, std::vector<long long>> mp;

public:
  int call_getMembersOfChat = 0;
  int call_getUserById = 0;
  long long last_chat_id;
  long long last_user_id;
  std::optional<User> mock_user;

  void setChatMembers(int chat_id, std::vector<long long> ids) {
    mp[chat_id] = ids;
  }

  std::optional<User> getUserById(long long otherUserId) override {
    ++call_getUserById;
    last_user_id = otherUserId;
    return mock_user;
  }

  std::vector<UserId> getMembersOfChat(long long chat_id) override {
    ++call_getMembersOfChat;
    last_chat_id = chat_id;
    return mp[chat_id];
  }
};

#endif // MOCKNETWORKMANAGER_H
