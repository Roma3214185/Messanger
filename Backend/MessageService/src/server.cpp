#include "messageservice/server.h"

#include "entities/RequestDTO.h"
#include "entities/UserMessage.h"
#include "interfaces/IRabitMQClient.h"
#include "messageservice/controller.h"
#include "messageservice/dto/GetMessagePack.h"
#include "messageservice/managers/JwtUtils.h"

namespace {

void sendResponse(crow::response &res, int code, const std::string &text) {
  LOG_INFO("Send responce code {} and body {}", code, text);
  res.code = code;
  res.write(text);
  res.end();
}

}  // namespace

Server::Server(crow::SimpleApp &app, int port, Controller *controller)
    : app_(app), port_(port), controller_(controller) {
  handleRoutes();
}

void Server::handleRoutes() {
  handleGetMessage();
  handleGetMessagesFromChat();
  handleUpdateMessage();
  handleDeleteMessage();
}

void Server::handleGetMessagesFromChat() {  // todo: chat/<string>/messages
  CROW_ROUTE(app_, "/messages/<string>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req, crow::response &res, const std::string &chat_id_str) {
            PROFILE_SCOPE();
            auto [code, body] = controller_->getMessagesFromChat(utils::getDTO(req, "messages/<chat_id>"), chat_id_str);
            sendResponse(res, code, body);
          });
}

void Server::handleGetMessage() {
  CROW_ROUTE(app_, "/message/<string>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req, crow::response &res, const std::string &message_id_str) {
            PROFILE_SCOPE();
            auto [code, body] = controller_->getMessageById(message_id_str);
            sendResponse(res, code, body);
          });
}

void Server::run() {
  LOG_INFO("[Message server is started on port '{}'", port_);
  app_.port(port_).multithreaded().run();
}

void Server::handleUpdateMessage() {
  CROW_ROUTE(app_, "/messages/<string>")
      .methods(crow::HTTPMethod::PUT)(
          [this](const crow::request &req, crow::response &res, const std::string &message_id_str) {
            PROFILE_SCOPE();
            auto [code, body] = controller_->updateMessage(utils::getDTO(req, "message/string"), message_id_str);
            sendResponse(res, code, body);
          });
}

void Server::handleDeleteMessage() {
  CROW_ROUTE(app_, "/messages/<string>")
      .methods(crow::HTTPMethod::DELETE)(
          [this](const crow::request &req, crow::response &res, const std::string &message_id_str) {
            PROFILE_SCOPE();
            auto [code, body] = controller_->deleteMessage(utils::getDTO(req, "message/string"), message_id_str);
            sendResponse(res, code, body);
          });
}
