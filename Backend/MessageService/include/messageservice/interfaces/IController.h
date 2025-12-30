#ifndef ICONTROLLER_H
#define ICONTROLLER_H

#include <vector>

#include "entities/RequestDTO.h"

class Message;
class MessageStatus;
class GetMessagePack;

using Responce = std::pair<int, std::string>;

class IController {
  public:
    virtual std::vector<Message> getMessages(const GetMessagePack&) = 0;
    virtual std::vector<MessageStatus> getMessagesStatus(const std::vector<Message>&, long long receiver_id) = 0;
    virtual Responce updateMessage(const RequestDTO& request_pack, const std::string& message_id_str) = 0;
    virtual Responce deleteMessage(const RequestDTO& request_pack, const std::string& message_id_str) = 0;
};

#endif // ICONTROLLER_H
