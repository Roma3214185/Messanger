#ifndef BACKEND_CHATSERVICE_SRC_SERVER_SERVER_H_
#define BACKEND_CHATSERVICE_SRC_SERVER_SERVER_H_

#include <crow/crow.h>
#include <memory>

#include "Controller/controller.h"
#include "DataBase/database.h"
#include "ChatManager/chatmanager.h"

using ControllerPtr = std::unique_ptr<Controller>;

class Server {
 public:
  Server(const int port, ChatManager* manager);
  void run();

 private:
  void initRoutes();

  int port_;
  ChatManager* manager_;
  crow::SimpleApp app_;
  ControllerPtr controller_;
};

#endif  // BACKEND_CHATSERVICE_SRC_SERVER_SERVER_H_
