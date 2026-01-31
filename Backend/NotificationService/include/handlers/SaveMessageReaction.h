#ifndef SAVEMESSAGEREACTION_H
#define SAVEMESSAGEREACTION_H

#include "utils.h"
#include "entities/Reaction.h"
#include "interfaces/IMessageHandler.h"
#include "notificationservice/managers/notificationmanager.h"

class SaveMessageReactionHandler : public IMessageHandler {
 public:
  void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket,
                 NotificationManager &manager);
};

#endif  // SAVEMESSAGEREACTION_H
