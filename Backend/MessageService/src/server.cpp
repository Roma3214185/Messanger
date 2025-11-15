#include "messageservice/server.h"

#include "messageservice/controller.h"
#include "RabbitMQClient.h"
#include "interfaces/IRabitMQClient.h"
#include "messageservice/managers/JwtUtils.h"
#include "messageservice/controller.h"

void sendResponse(crow::response& res, int code, const std::string& text) {
  res.code = code;
  res.write(text);
  res.end();
}

std::string extractToken(const crow::request& req) { return req.get_header_value("Authorization"); }

int getLimit(const crow::request& req) {
  return req.url_params.get("limit") ? std::stoi(req.url_params.get("limit")) : INT_MAX;
}

int getBeforeId(const crow::request& req) {
  return req.url_params.get("before_id") ? std::stoi(req.url_params.get("before_id")) : 0;
}

crow::json::wvalue formMessageListJson(const std::vector<Message>& messages,
                                       const std::vector<MessageStatus>& messages_status) {
  assert(messages.size() == messages_status.size());
  crow::json::wvalue res = crow::json::wvalue::list();
  int i   = 0;
  for (const auto& msg : messages) {
    auto jsonObject            = to_crow_json(msg);
    jsonObject["readed_by_me"] = messages_status[i].is_read;
    res[i++] = std::move(jsonObject);
  }

  return res;
}

Server::Server(int port, Controller* controller) : port_(port), controller_(controller) {
  handleRoutes();
}

std::optional<int> getUserIdFromToken(const std::string& token) {
  return JwtUtils::verifyTokenAndGetUserId(token);
}

void Server::handleRoutes() { handleGetMessagesFromChat(); }

void Server::handleGetMessagesFromChat() {
  CROW_ROUTE(app_, "/messages/<int>")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, int chat_id) {
            PROFILE_SCOPE("/message/<int>");
            std::string token = extractToken(req);
            std::optional<int> optional_user_id = getUserIdFromToken(token);
            if(!optional_user_id){
              sendResponse(res, 400, "Invalid token");
              return;
            }

            int user_id = *optional_user_id;

            GetMessagePack pack { .before_id = getBeforeId(req), .chat_id = chat_id,
                                .limit = getLimit(req), .user_id = user_id };

            auto messages = controller_->getMessages(pack);
            auto messages_status = controller_->getMessagesStatus(messages, user_id);
            auto json_messages = formMessageListJson(messages, messages_status);
            sendResponse(res, 200, json_messages.dump());

            LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
          });
}

void Server::run() {
  spdlog::info("[Message server is started on port '{}'", port_);
  app_.port(port_).multithreaded().run();
}
