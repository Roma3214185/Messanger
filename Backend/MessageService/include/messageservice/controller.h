#ifndef BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
#define BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_

#include <crow.h>

#include <QtSql>
#include <mutex>
#include <string>
#include <unordered_map>

#include "threadpool.h"
#include "ProdConfigProvider.h"
#include "messageservice/interfaces/IController.h"

class Message;
class MessageManager;
class IRabitMQClient;
class GetMessagePack;
class MessageStatus;
class IThreadPool;

class Controller : public IController {
 public:
  Controller(IRabitMQClient* mq_client,
              MessageManager* manager, IThreadPool* pool, IConfigProvider* provider = &ProdConfigProvider::instance());
  std::vector<Message> getMessages(const GetMessagePack&) override;
  std::vector<MessageStatus> getMessagesStatus(const std::vector<Message>& messages, long long receiver_id) override;

 protected:
  void subscribeToSaveMessage();
  void subscribeToSaveMessageStatus();
  virtual void       handleSaveMessage(const std::string& payload);
  virtual void       handleSaveMessageStatus(const std::string& payload);

  std::mutex       socket_mutex_;
  MessageManager*  manager_;
  IRabitMQClient*  mq_client_;
  IConfigProvider* provider_;
  IThreadPool*     pool_;
};

#endif  // BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
