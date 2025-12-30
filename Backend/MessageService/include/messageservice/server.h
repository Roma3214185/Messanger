#ifndef BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
#define BACKEND_MESSAGESERVICE_SERVER_SERVER_H_

#include <crow.h>
#include <memory>
#include "managers/MessageManager.h"
#include "ProdConfigProvider.h"

class IRabitMQClient;
class IController;

class Server {
 public:
  using OptionalId = std::optional<long long>;
  Server(crow::SimpleApp& app, int port, IController* controller, IConfigProvider* provider = &ProdConfigProvider::instance());
  void run();

 protected:
  static crow::json::wvalue formMessageListJson(const std::vector<Message>& messages,
                                                  const std::vector<MessageStatus>& messages_status);

 private:
  virtual void onGetMessagesFromChat(const crow::request& req, long long chat_id, crow::response& res);
  void handleGetMessagesFromChat();
  void handleRoutes();
  virtual OptionalId getUserIdFromToken(const std::string& token);
  void handleUpdateMessage();
  void handleDeleteMessage();

  crow::SimpleApp& app_;
  int             port_;
  IController*   controller_;
  IConfigProvider* provider_;
};

#endif  // BACKEND_MESSAGESERVICE_SERVER_SERVER_H_
