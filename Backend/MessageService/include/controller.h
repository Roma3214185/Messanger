#ifndef BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
#define BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_

#include <crow.h>

#include <QtSql>
#include <mutex>
#include <string>
#include <unordered_map>

#include "ThreadPool.h"
#include "ProdConfigProvider.h"

class Message;
class MessageManager;
class RabbitMQClient;

class Controller {
 public:
  Controller(crow::SimpleApp& app, RabbitMQClient* mq_client,
              MessageManager* manager, IConfigProvider* provider = &ProdConfigProvider::instance());
  void getMessagesFromChat(const crow::request& req, int chat_id, crow::response& res);

 private:
  void               subscribeSaveMessageStatus();
  void               subscribeSaveMessage();
  void               onSendMessage(Message message);
  void               handleSaveMessage(const std::string& payload);
  void               handleSaveMessageStatus(const std::string& payload);
  crow::json::wvalue formMessageListJson(const std::vector<Message>& messages, int current_user_id);

  std::mutex       socket_mutex_;
  crow::SimpleApp& app_;
  MessageManager*  manager_;
  RabbitMQClient*  mq_client_;
  IConfigProvider* provider_;
  ThreadPool       pool_{4};
};

#endif  // BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
