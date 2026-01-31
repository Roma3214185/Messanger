#ifndef SAVEMESSAGEREACTION_H
#define SAVEMESSAGEREACTION_H

#include "entities/Reaction.h"
#include "interfaces/IMessageHandler.h"
#include "notificationservice/managers/notificationmanager.h"
#include "utils.h"

class SaveMessageReactionHandler : public IMessageHandler {
 public:
  void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket, NotificationManager &manager);
};

#endif  // SAVEMESSAGEREACTION_H
