#ifndef SENDMESSAGEHANDLER_H
#define SENDMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class IPublisher;

class SendMessageHandler : public IMessageHandler {
 public:
  SendMessageHandler(IPublisher* publisher);
  void handle(const crow::json::rvalue& message, [[maybe_unused]] const std::shared_ptr<ISocket>& socket) override;

 private:
  IPublisher* publisher_;
};

#endif  // SENDMESSAGEHANDLER_H
