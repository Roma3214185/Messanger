#ifndef BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
#define BACKEND_MESSAGESERVICE_SERVER_SERVER_H_

#include <crow.h>
#include <memory>

#include "managers/MessageManager.h"

class IRabitMQClient;
class Controller;

class Server {
 public:
  Server(int port, Controller* controller);
  void run();

 private:
  void handleRoutes();
  void handleGetMessagesFromChat();

  crow::SimpleApp app_;
  int             port_;
  Controller*   controller_;
};

#endif  // BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
