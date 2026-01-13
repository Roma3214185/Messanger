#ifndef BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
#define BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_

#include <string>
#include <unordered_map>
#include <vector>

class Message;
class MessageManager;
class IRabitMQClient;
class GetMessagePack;
class MessageStatus;
class IThreadPool;
class RequestDTO;

using StatusCode = int;
using ResponceBody = std::string;
using Response = std::pair<StatusCode, ResponceBody>;

class Controller {
 public:
  Controller(IRabitMQClient *mq_client, MessageManager *manager, IThreadPool *pool);

  Response updateMessage(const RequestDTO &request_pack, const std::string &message_id_str);
  Response deleteMessage(const RequestDTO &request_pack, const std::string &message_id_str);
  Response getMessageById(const std::string &message_id_str);
  Response getMessagesFromChat(const RequestDTO &request_pack, const std::string &chat_id_str);

 protected:
  virtual void handleSaveMessage(const std::string &payload);
  virtual void handleSaveMessageStatus(const std::string &payload);
  void subscribeToSaveMessage();
  void subscribeToSaveMessageStatus();
  void subscribeToSaveMessageReaction();
  void subscribeToDeleteMessageReaction();
  void  onDeleteMessageReaction(const std::string &payload);
  void  onSaveMessageReaction(const std::string &payload);

 private:
  std::vector<Message> getMessages(const GetMessagePack &);
  std::vector<MessageStatus> getMessagesStatus(const std::vector<Message> &messages, long long receiver_id);
  std::vector<MessageStatus> getReadedMessageStatuses(long long message_id);
  std::optional<long long> getUserIdFromToken(const std::string &token);

  MessageManager *manager_;
  IRabitMQClient *mq_client_;
  IThreadPool *pool_;
};

#endif  // BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
