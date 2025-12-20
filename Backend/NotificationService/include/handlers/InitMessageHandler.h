#ifndef INITMESSAGEHANDLER_H
#define INITMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class InitMessageHandler : public IMessageHandler {
  public:
    void handle(const crow::json::rvalue& message,
                std::shared_ptr<ISocket> socket,
                NotificationManager& manager) override {

      if (!message.has("user_id")) {
        LOG_ERROR("[init] No user_id");
        return;
      }

      long long user_id = std::stoll(message["user_id"].s());
      manager.userConnected(user_id, socket);
      LOG_INFO("[init] Socket registered for userId '{}'", user_id);
    }
};

#endif // INITMESSAGEHANDLER_H
