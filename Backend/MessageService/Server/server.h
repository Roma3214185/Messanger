#ifndef BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
#define BACKEND_MESSAGESERVICE_SERVER_SERVER_H_

#include <memory>

#include <crow.h>

#include "MessageManager/MessageManager.h"
#include "Controller/controller.h"

using ControllerPtr = std::unique_ptr<Controller>;

class Server {
 public:
  Server(int port, MessageManager* message_manager, RabbitMQClient* mq_client);
  void run();

 private:
  void handleRoutes();
  void handleGetMessagesFromChat();


  crow::SimpleApp app_;
  int port_;
  ControllerPtr controller_;
};

#endif  // BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
