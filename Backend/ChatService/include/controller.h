#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <crow.h>

#include "chatmanager.h"
#include "database.h"

class NetworkManager;

class Controller {
 public:
  Controller(crow::SimpleApp& app, ChatManager* manager, NetworkManager* network_manager);
  void createPrivateChat(const crow::request& req, crow::response& res);
  void getAllChats(const crow::request& req, crow::response& res);
  void getAllChatsById(const crow::request& req, crow::response& res, int chat_id);
  void getAllChatMembers(const crow::request& req, crow::response& res, int chat_id);

 private:
  crow::SimpleApp& app_;
  ChatManager*     manager_;
  NetworkManager* network_manager_;
};

#endif  // CONTROLLER_H
