#ifndef SENDMESSAGEHANDLER_H
#define SENDMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class SendMessageHandler : public IMessageHandler {
  public:
    void handle(const crow::json::rvalue& message,
                std::shared_ptr<ISocket>,
                NotificationManager& manager) override {

      auto msg = from_crow_json(message);
      manager.onSendMessage(msg);
      LOG_INFO("[send_message] Message processed");
    }
};

#endif // SENDMESSAGEHANDLER_H
