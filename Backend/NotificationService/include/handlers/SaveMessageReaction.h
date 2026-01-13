#ifndef SAVEMESSAGEREACTION_H
#define SAVEMESSAGEREACTION_H

#include "interfaces/IMessageHandler.h"
#include "Utils.h"
#include "entities/Reaction.h"
#include "notificationservice/managers/notificationmanager.h"

class SaveMessageReactionHandler : public IMessageHandler {
  public:
    void handle(const crow::json::rvalue &message, const std::shared_ptr<ISocket> &socket,
                NotificationManager &manager) override {
      if(auto reaction = utils::entities::parseReaction(message); reaction.has_value()) {  //todo: u can here fully implement SaveMessageReaction in mq
          manager.saveReaction(*reaction);
      }
    }
};

#endif // SAVEMESSAGEREACTION_H
