#ifndef DELETEMESSAGERESPONCE_H
#define DELETEMESSAGERESPONCE_H

#include "JsonService.h"
#include "interfaces/ISocketResponceHandler.h"
#include "managers/TokenManager.h"
#include "usecases/messageusecase.h"

class DeleteMessageResponceHandler : public ISocketResponceHandler {
  TokenManager*   token_manager_;
  MessageUseCase* message_use_case_;

 public:
  DeleteMessageResponceHandler(TokenManager* token_manager, MessageUseCase* message_use_case)
      : token_manager_(token_manager), message_use_case_(message_use_case) {}

  void handle(const QJsonObject& json_object) override {
    auto new_message = JsonService::getMessageFromJson(json_object);
    int  current_id  = token_manager_->getCurrentUserId();
    if (current_id == new_message.sender_id) new_message.is_mine = true;
    new_message.status_sended = true;
    message_use_case_->deleteMessage(new_message);
  }
};

#endif  // DELETEMESSAGERESPONCE_H
