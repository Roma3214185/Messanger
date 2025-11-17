#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <crow.h>

#include "ProdConfigProvider.h"

class NetworkFacade;
class IConfigProvider;
class IChatManager;
class User;
class Chat;

class ChatController {
 public:
  ChatController(IChatManager* manager,
              NetworkFacade* network_facade, IConfigProvider* provider = &ProdConfigProvider::instance());
  void createPrivateChat(const crow::request& req, crow::response& res);
  void getAllChats(const crow::request& req, crow::response& res);
  void getChat(const crow::request& req, crow::response& res, int chat_id);
  void getAllChatMembers(const crow::request& req, crow::response& res, int chat_id);

 private:
  std::optional<int> authorizeUser(const crow::request& req, crow::response& res);
  void sendError(crow::response& res, int status, const std::string& message, const std::string& log_msg);
  crow::json::wvalue buildChatJson(const Chat& chat, int current_user_id);
  virtual std::optional<User> getUserById(int id);
  std::optional<int>  autoritize(const crow::request& req);

  IChatManager*     manager_;
  NetworkFacade* network_facade_;
  IConfigProvider* provider_;
};

#endif  // CHATCONTROLLER_H
