#ifndef NEWMESSAGERESPONCEHANDLER_H
#define NEWMESSAGERESPONCEHANDLER_H

#include "JsonService.h"
#include "interfaces/ISocketResponceHandler.h"
#include "managers/TokenManager.h"
#include "usecases/messageusecase.h"

class NewMessageResponceHandler : public ISocketResponceHandler {
  TokenManager *token_manager_;
  MessageUseCase *message_use_case_;

 public:
  NewMessageResponceHandler(TokenManager *token_manager, MessageUseCase *message_use_case)
      : token_manager_(token_manager), message_use_case_(message_use_case) {}

  void handle(const QJsonObject &json_object) override {
    auto new_message = JsonService::getMessageFromJson(json_object);  // todo: readed by me here and maybe status_sended
                                                                      // in JsonService
    int current_id = token_manager_->getCurrentUserId();
    if (current_id == new_message.sender_id) new_message.is_mine = true;
    new_message.status_sended = true;
    message_use_case_->addMessageToChat(new_message);  // how to update message_list_view_ (?)
  }
};

#endif  // NEWMESSAGERESPONCEHANDLER_H
