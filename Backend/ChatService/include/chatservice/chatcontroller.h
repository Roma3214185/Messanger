#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <string>
#include <vector>

class NetworkFacade;
class IConfigProvider;
class IChatManager;
class User;
class Chat;
class RequestDTO;
struct ReactionInfo;

using StatusCode = int;
using ResponceBody = std::string;
using Response = std::pair<StatusCode, ResponceBody>;

class ChatController {
 public:
  ChatController(IChatManager *manager, NetworkFacade *network_facade);

  Response createPrivateChat(const RequestDTO &req);
  Response getAllChats(const RequestDTO &req);
  Response getChat(const RequestDTO &req, const std::string &chat_id_str);
  Response getAllChatMembers(const RequestDTO &req, const std::string &chat_id_str);

 private:
  std::optional<long long> authorizeUser(const RequestDTO &req);
  virtual std::optional<User> getUserById(long long id);
  std::optional<long long> autoritize(const std::string &token);
  std::vector<ReactionInfo> getReactionOfChat(long long chat_id);

  IChatManager *manager_;
  NetworkFacade *network_facade_;
};

#endif  // CHATCONTROLLER_H
