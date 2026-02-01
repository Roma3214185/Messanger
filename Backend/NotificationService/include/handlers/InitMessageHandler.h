#ifndef INITMESSAGEHANDLER_H
#define INITMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class InitMessageHandler : public IMessageHandler {
 public:
  void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket, NotificationManager &manager);
};

#endif  // INITMESSAGEHANDLER_H
