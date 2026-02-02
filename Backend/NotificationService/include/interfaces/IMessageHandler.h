#ifndef IMESSAGEHANDLER_H
#define IMESSAGEHANDLER_H

#include <crow.h>

class ISocket;

class IMessageHandler {
 public:
  virtual ~IMessageHandler() = default;
  virtual void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket) = 0;
};

#endif  // IMESSAGEHANDLER_H
