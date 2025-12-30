#ifndef MOCKCONTROLLER_H
#define MOCKCONTROLLER_H

#include "messageservice/interfaces/IController.h"
#include "messageservice/dto/GetMessagePack.h"
#include "entities/Message.h"
#include "entities/MessageStatus.h"

struct MockController : public IController {
  std::vector<Message> mock_messages;
  GetMessagePack mock_messages_pack;
  std::vector<MessageStatus> mock_messages_status;
  Responce mock_responce;

   std::vector<Message> getMessages(const GetMessagePack& pack) override {
    mock_messages_pack = pack;
    return mock_messages;
   }

   std::vector<MessageStatus> getMessagesStatus(const std::vector<Message>&, long long receiver_id) override {
     return mock_messages_status;
   }

   Responce updateMessage(const RequestDTO& request_pack, const std::string& message_id_str) override {
     return mock_responce;
   }

   Responce deleteMessage(const RequestDTO& request_pack, const std::string& message_id_str) override {
     return mock_responce;
   }
};

#endif // MOCKCONTROLLER_H
