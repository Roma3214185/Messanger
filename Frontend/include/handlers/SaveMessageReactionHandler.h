#ifndef SAVEMESSAGEREACTIONHANDLER_H
#define SAVEMESSAGEREACTIONHANDLER_H

class NewMessageResponceHandler : public ISocketResponceHandler {
  EntityFactory *entity_factory_;
  MessageUseCase *message_use_case_;

 public:
  NewMessageResponceHandler(EntityFactory *entity_factory, MessageUseCase *message_use_case)
      : entity_factory_(entity_factory), message_use_case_(message_use_case) {}

  void handle(const QJsonObject &json_object) override {
    auto message = entity_factory_->getMessageFromJson(json_object);
    message_use_case_->addMessageToChat(message);
  }
};

#endif  // SAVEMESSAGEREACTIONHANDLER_H
