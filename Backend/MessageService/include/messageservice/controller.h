#ifndef BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
#define BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "QueueSubscriber.h"
#include "QueuePublisher.h"

class Message;
class IMessageCommandService;
class IMessageQueryService;
class IEventBus;
class GetMessagePack;
class MessageStatus;
class IThreadPool;
class RequestDTO;
struct ReactionInfo;

using StatusCode = int;
using ResponceBody = std::string;
using Response = std::pair<StatusCode, ResponceBody>;

class Controller {
 public:
  Controller(IEventBus *mq_client, IMessageCommandService* command_manager, IMessageQueryService *query_manager, IThreadPool *pool);

  Response updateMessage(const RequestDTO &request_pack, const std::string &message_id_str);
  Response deleteMessage(const RequestDTO &request_pack, const std::string &message_id_str);
  Response getMessageById(const std::string &message_id_str);
  Response getMessagesFromChat(const RequestDTO &request_pack, const std::string &chat_id_str);
  Response getReaction(const RequestDTO &request_pack, const std::string &reaction_id_str);
  Response setup();

 protected:
  virtual void handleSaveMessage(const std::string &payload);
  virtual void handleSaveMessageStatus(const std::string &payload);
  void handleDeleteMessageReaction(const std::string &payload);
  void handleSaveMessageReaction(const std::string &payload);

  void subscribeAll();

 private:
  std::vector<Message> getMessages(const GetMessagePack &);
  std::vector<MessageStatus> getMessagesStatus(const std::vector<Message> &messages, long long receiver_id);
  std::vector<MessageStatus> getReadedMessageStatuses(long long message_id);
  std::optional<long long> getUserIdFromToken(const std::string &token);
  std::optional<std::vector<ReactionInfo>> loadReactions();

  IMessageCommandService *command_manager_;
  IMessageQueryService *query_manager_;
  IThreadPool *pool_;
  QueueSubscriber subscriber_;
  QueuePublisher publisher_;
};

#endif  // BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
