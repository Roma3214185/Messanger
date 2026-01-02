#include "chatservice/chatserver.h"

#include "chatservice/chatcontroller.h"
#include "Debug_profiling.h"
#include "entities/RequestDTO.h"

namespace {

void sendResponse(crow::response& res, int code, const std::string& text) {
  LOG_INFO("Send responce code {} and body {}", code, text);
  res.code = code;
  res.write(text);
  res.end();
}

}  // namespace

ChatServer::ChatServer(crow::SimpleApp& app, int port, ChatController* controller)
    : app_(app), port_(port), controller_(controller) {
  initRoutes();
}

void ChatServer::initRoutes() {
  handleCreatingPrivateChat();
  handleGetChat();
  handleGetAllChats();
  handleGetAllChatsMembers();
}

void ChatServer::run() {
  LOG_INFO("Chat microservice is running on port {}", port_);
  app_.port(port_).multithreaded().run();
}

void ChatServer::handleCreatingPrivateChat() {
  CROW_ROUTE(app_, "/chats/private")
      .methods(crow::HTTPMethod::POST)([&](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE();
        auto [code, body] = controller_->createPrivateChat(utils::getDTO(req, "/auth/me"));
        sendResponse(res, code, body);
      });
}

void ChatServer::handleGetAllChats() {
  CROW_ROUTE(app_, "/chats")
      .methods(crow::HTTPMethod::GET)([&](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE();
        auto [code, body] = controller_->getAllChats(utils::getDTO(req, "/chats"));
        sendResponse(res, code, body);
      });
}

void ChatServer::handleGetChat() {
  CROW_ROUTE(app_, "/chats/<string>")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, const std::string& chat_id_str) {
            PROFILE_SCOPE();
            auto [code, body] = controller_->getChat(utils::getDTO(req, "/chats/id"), chat_id_str);
            sendResponse(res, code, body);
          });
}

void ChatServer::handleGetAllChatsMembers() {
  CROW_ROUTE(app_, "/chats/<string>/members")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, const std::string& chat_id_str) {
            PROFILE_SCOPE();
            auto [code, body] = controller_->getAllChatMembers(utils::getDTO(req, "/chats/id/members"), chat_id_str);
            sendResponse(res, code, body);
          });
}
