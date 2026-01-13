#ifndef DELETEMESSAGERESPONCE_H
#define DELETEMESSAGERESPONCE_H

#include "JsonService.h"
#include "interfaces/ISocketResponceHandler.h"
#include "usecases/messageusecase.h"

class DeleteMessageResponceHandler : public ISocketResponceHandler {
  EntityFactory *entity_factory_;
  MessageUseCase *message_use_case_;

 public:
  DeleteMessageResponceHandler(EntityFactory *entity_factory, MessageUseCase *message_use_case)
      : entity_factory_(entity_factory), message_use_case_(message_use_case) {}

  void handle(const QJsonObject &json_object) override {
    message_use_case_->deleteMessage(entity_factory_->getMessageFromJson(json_object));
  }
};

#endif  // DELETEMESSAGERESPONCE_H
