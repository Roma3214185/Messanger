#ifndef MOCKCONTROLLER_H
#define MOCKCONTROLLER_H

#include "messageservice/interfaces/IController.h"
#include "messageservice/dto/GetMessagePack.h"

struct MockController : public IController {
   std::vector<Message> mock_messages;
  GetMessagePack mock_messages_pack;
   std::vector<MessageStatus> mock_messages_status;

   std::vector<Message> getMessages(const GetMessagePack& pack) override {
    mock_messages_pack = pack;
    return mock_messages;
   }

   std::vector<MessageStatus> getMessagesStatus(const std::vector<Message>&, int receiver_id) override {
     return mock_messages_status;
   }
};

#endif // MOCKCONTROLLER_H
