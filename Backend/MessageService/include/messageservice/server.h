#ifndef BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
#define BACKEND_MESSAGESERVICE_SERVER_SERVER_H_

#include <crow.h>

class Controller;

class Server {
 public:
  using OptionalId = std::optional<long long>;
  Server(crow::SimpleApp& app, int port, Controller* controller);
  void run();

 private:
  void handleGetMessagesFromChat();
  void handleRoutes();
  void handleGetMessage();
  void handleUpdateMessage();
  void handleDeleteMessage();

  crow::SimpleApp& app_;
  int             port_;
  Controller*   controller_;
};

#endif  // BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
