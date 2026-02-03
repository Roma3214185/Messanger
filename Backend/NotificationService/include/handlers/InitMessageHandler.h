#ifndef INITMESSAGEHANDLER_H
#define INITMESSAGEHANDLER_H

#include "interfaces/IMessageHandler.h"

class IUserSocketRepository;

class InitMessageHandler : public IMessageHandler {
 public:
  InitMessageHandler(IUserSocketRepository* socket_repository);
  void handle(const crow::json::rvalue& message, const std::shared_ptr<ISocket>& socket) override;

 private:
  IUserSocketRepository* socket_repository_;
};

#endif  // INITMESSAGEHANDLER_H
