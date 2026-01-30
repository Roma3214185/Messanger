#ifndef DELETEMESSAGEREACTION_H
#define DELETEMESSAGEREACTION_H

#include "Utils.h"
#include "entities/Reaction.h"
#include "interfaces/IMessageHandler.h"
#include "notificationservice/managers/notificationmanager.h"

class DeleteMessageReactionHandler : public IMessageHandler {
 public:
  void handle(const crow::json::rvalue &message,
              const std::shared_ptr<ISocket> &socket,  // todo: in handle ONLY crow::json::rvalue
              NotificationManager &manager) override;
};

#endif  // DELETEMESSAGEREACTION_H
