#ifndef SOCKETHANDLERSREPOSITOTY_H
#define SOCKETHANDLERSREPOSITOTY_H

#include "handlers/MessageHanldlers.h"
#include "interfaces/IMessageHandler.h"
#include "notificationservice/managers/NotificationOrchestrator.h"

using SocketHandlers = std::unordered_map<std::string, std::shared_ptr<IMessageHandler>>;

class NotificationOrchestrator;

class SocketHandlersRepository {
 public:
  void setHandlers(SocketHandlers &&handlers);

  void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket);

 private:
  SocketHandlers handlers_;
};

#endif  // SOCKETHANDLERSREPOSITOTY_H
