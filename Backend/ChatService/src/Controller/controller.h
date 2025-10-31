#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <crow/crow.h>

#include "database.h"
#include "ChatManager/chatmanager.h"

class Controller {
 public:
  Controller(crow::SimpleApp& app, ChatManager* manager);
  void handleRoutes();

 private:
  void handleCreatingPrivateChat();
  void handleGetAllChats();
  void handleGetAllChatsById();
  void handleGetAllChatsMembers();

  crow::SimpleApp& app_;
  ChatManager* manager_;
};

#endif  // CONTROLLER_H
