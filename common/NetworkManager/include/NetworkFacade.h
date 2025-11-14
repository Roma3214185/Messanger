#ifndef NETWORKFACADE_H
#define NETWORKFACADE_H

#include "interfaces/IChatNetworkManager.h"
#include "interfaces/IUserNetworkManager.h"
#include "interfaces/IMessageNetworkManager.h"

class NetworkFacade {
  public:
    NetworkFacade(IUserNetworkManager* user,
                  IMessageNetworkManager* msg,
                  IChatNetworkManager* chat)
        : user_(user),
        message_(msg),
        chat_(chat) {}

    IUserNetworkManager& user()    { return *user_; }
    IMessageNetworkManager& msg()  { return *message_; }
    IChatNetworkManager& chat()    { return *chat_; }

  private:
    IUserNetworkManager*  user_;
    IMessageNetworkManager* message_;
    IChatNetworkManager*  chat_;
};

class NetworkFactory {
  public:
    static NetworkFacade create(INetworkManagerBase* base) {

      auto* u = dynamic_cast<IUserNetworkManager*>(base);
      auto* m = dynamic_cast<IMessageNetworkManager*>(base);
      auto* c = dynamic_cast<IChatNetworkManager*>(base);

      return NetworkFacade(u, m, c);
    }
};

#endif // NETWORKFACADE_H
