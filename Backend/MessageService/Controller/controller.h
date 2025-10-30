#ifndef BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
#define BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_

#include <mutex>
#include <unordered_map>
#include <string>

#include <crow.h>
#include <QtSql>

#include "ThreadPool.h"

class Message;
class MessageManager;
class RabbitMQClient;

class Controller {
 public:
  Controller(crow::SimpleApp& app, RabbitMQClient* mq_client,
             MessageManager* manager);
  void handleRoutes();

 private:
  void subscribeSaveMessage();
  void handleGetMessagesFromChat();
  void onSendMessage(Message message);
  std::string getToken(const crow::request& req);
  void handleSaveMessage(const std::string& payload);
  void handleSaveMessageStatus(const std::string& payload);

  std::mutex socket_mutex_;
  crow::SimpleApp& app_;
  MessageManager* manager_;
  RabbitMQClient* mq_client_;
  ThreadPool pool_{4};
};

#endif  // BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
