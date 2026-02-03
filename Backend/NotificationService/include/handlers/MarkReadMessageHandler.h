#ifndef MARKREADMESSAGEHANDLER_H
#define MARKREADMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class IPublisher;

class MarkReadMessageHandler : public IMessageHandler {
 public:
  MarkReadMessageHandler(IPublisher* publisher);
  void handle(const crow::json::rvalue& message, const std::shared_ptr<ISocket>& socket);

 private:
  IPublisher* publisher_;
};

#endif  // MARKREADMESSAGEHANDLER_H
