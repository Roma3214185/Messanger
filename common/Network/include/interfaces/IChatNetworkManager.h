#ifndef ICHATNETWORKMANAGER_H
#define ICHATNETWORKMANAGER_H

#include <vector>

using UserId = long long;

class IChatNetworkManager {
 public:
  virtual ~IChatNetworkManager() = default;
  virtual std::vector<UserId> getMembersOfChat(long long chat_id) = 0;
};

class ProxyClient;

class ChatNetworkManager : public IChatNetworkManager {
 public:
  ChatNetworkManager(ProxyClient* proxy);
  std::vector<UserId> getMembersOfChat(long long chat_id) override;

 private:
  ProxyClient* proxy_;
};

#endif  // ICHATNETWORKMANAGER_H
