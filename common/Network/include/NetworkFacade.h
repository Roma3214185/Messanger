#ifndef NETWORKFACADE_H
#define NETWORKFACADE_H

#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"

class INetworkFacade {
 public:
  virtual IUserNetworkManager& users() = 0;
  virtual IMessageNetworkManager& messages() = 0;
  virtual IChatNetworkManager& chats() = 0;
  virtual ~INetworkFacade() = default;
};

class NetworkFacade : public INetworkFacade {
 public:
  NetworkFacade(ProxyClient* proxy) : users_(proxy), messages_(proxy), chats_(proxy) {}

  IUserNetworkManager& users() override { return users_; }
  IMessageNetworkManager& messages() override { return messages_; }
  IChatNetworkManager& chats() override { return chats_; }

 private:
  UserNetworkManager users_;
  MessageNetworkManager messages_;
  ChatNetworkManager chats_;
};

#endif  // NETWORKFACADE_H
