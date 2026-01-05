#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include "ProdConfigProvider.h"

class NetworkFacade;
class IConfigProvider;
class IChatManager;
class User;
class Chat;
class RequestDTO;

using StatusCode   = int;
using ResponceBody = std::string;
using Response     = std::pair<StatusCode, ResponceBody>;

class ChatController {
 public:
  ChatController(IChatManager*    manager,
                 NetworkFacade*   network_facade,
                 IConfigProvider* provider = &ProdConfigProvider::instance());

  Response createPrivateChat(const RequestDTO& req);
  Response getAllChats(const RequestDTO& req);
  Response getChat(const RequestDTO& req, const std::string& chat_id_str);
  Response getAllChatMembers(const RequestDTO& req, const std::string& chat_id_str);

 private:
  std::optional<long long>    authorizeUser(const RequestDTO& req);
  virtual std::optional<User> getUserById(long long id);
  std::optional<long long>    autoritize(const std::string& token);

  IChatManager*    manager_;
  NetworkFacade*   network_facade_;
  IConfigProvider* provider_;
};

#endif  // CHATCONTROLLER_H
