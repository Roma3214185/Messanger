#ifndef NEWMESSAGERESPONCEHANDLER_H
#define NEWMESSAGERESPONCEHANDLER_H

#include "JsonService.h"
#include "interfaces/ISocketResponceHandler.h"
#include "managers/TokenManager.h"
#include "usecases/messageusecase.h"

class NewMessageResponceHandler : public ISocketResponceHandler {
  EntityFactory *entity_factory_;
  MessageUseCase *message_use_case_;

 public:
  NewMessageResponceHandler(EntityFactory *entity_factory, MessageUseCase *message_use_case)
      : entity_factory_(entity_factory), message_use_case_(message_use_case) {}

  void handle(const QJsonObject &json_object) override {
    auto [message, reactions] = entity_factory_->getMessageFromJson(json_object);
    message_use_case_->addMessageToChat(message);
    message_use_case_->saveReactionInfo(reactions);
  }
};

#endif  // NEWMESSAGERESPONCEHANDLER_H
