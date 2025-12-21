#ifndef ICONTROLLER_H
#define ICONTROLLER_H

#include <vector>

class Message;
class MessageStatus;
class GetMessagePack;

class IController {
  public:
    virtual std::vector<Message> getMessages(const GetMessagePack&) = 0;
    virtual std::vector<MessageStatus> getMessagesStatus(const std::vector<Message>&, long long receiver_id) = 0;
};

#endif // ICONTROLLER_H
