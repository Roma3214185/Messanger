#ifndef BACKEND_CHATSERVICE_SRC_SERVER_SERVER_H_
#define BACKEND_CHATSERVICE_SRC_SERVER_SERVER_H_

#include <crow.h>

#include <memory>

class ChatController;
class NetworkManager;

class ChatServer {
 public:
  ChatServer(crow::SimpleApp& app, int port, ChatController* controller);
  void run();

 private:
  void initRoutes();

  void handleCreatingPrivateChat();
  void handleGetAllChats();
  void handleGetChat();
  void handleGetAllChatsMembers();

  int              port_;
  crow::SimpleApp& app_;
  ChatController*  controller_;
};

#endif  // BACKEND_CHATSERVICE_SRC_SERVER_SERVER_H_
