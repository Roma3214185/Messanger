#ifndef BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
#define BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_

#include <mutex>
#include <unordered_map>
#include <string>

#include <crow.h>
#include <QtSql>

class Message;
class MessageManager;
class RabbitMQClient;

class Controller {
 public:
  Controller(crow::SimpleApp& app, RabbitMQClient* mq_client,
             MessageManager* manager);
  void handleRoutes();

 private:
  void handleGetMessagesFromChat();
  void subscribeToEntitySaving();
  void onSendMessage(Message message);
  std::string getToken(const crow::request& req);

  std::mutex socket_mutex_;
  crow::SimpleApp& app_;
  MessageManager* manager_;
  RabbitMQClient* mq_client_;
};

#endif  // BACKEND_MESSAGESERVICE_CONTROLLER_CONTROLLER_H_
