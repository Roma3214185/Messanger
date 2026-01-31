#ifndef SENDMESSAGEHANDLER_H
#define SENDMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class SendMessageHandler : public IMessageHandler {
 public:
  void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket,
            NotificationManager &manager) override;
};

#endif  // SENDMESSAGEHANDLER_H
