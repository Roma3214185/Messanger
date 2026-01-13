#ifndef SENDMESSAGEHANDLER_H
#define SENDMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class SendMessageHandler : public IMessageHandler {
 public:
  void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket,
              NotificationManager &manager) override {
    auto msg = utils::entities::from_crow_json(message);   //todo utils::parsePayload<Message>(message.dump())
    manager.onSendMessage(msg);
    LOG_INFO("[send_message] Message processed");
  }
};

#endif  // SENDMESSAGEHANDLER_H
