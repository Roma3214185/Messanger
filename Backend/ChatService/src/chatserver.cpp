#include "chatservice/chatserver.h"

#include "chatservice/chatcontroller.h"
#include "Debug_profiling.h"

ChatServer::ChatServer(crow::SimpleApp& app, int port, ChatController* controller)
    : app_(app), port_(port), controller_(controller) {
  initRoutes();
}

void ChatServer::initRoutes() {
  handleCreatingPrivateChat();
  handleGetAllChats();
  handleGetAllChatsById();
  handleGetAllChatsMembers();
}

void ChatServer::run() {
  LOG_INFO("Chat microservice is running on port {}", port_);
  app_.port(port_).multithreaded().run();
}

void ChatServer::handleCreatingPrivateChat() {
  CROW_ROUTE(app_, "/chats/private")
      .methods(crow::HTTPMethod::POST)([&](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("/auth/me");
        controller_->createPrivateChat(req, res);
        LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}

void ChatServer::handleGetAllChats() {
  CROW_ROUTE(app_, "/chats")
      .methods(crow::HTTPMethod::GET)([&](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("/chats");
        controller_->getAllChats(req, res);
        LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}

void ChatServer::handleGetAllChatsById() {
  CROW_ROUTE(app_, "/chats/<int>")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, int chat_id) {
            PROFILE_SCOPE("/chats/<int>");
            controller_->getAllChatsById(req, res, chat_id);
            LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
          });
}

void ChatServer::handleGetAllChatsMembers() {
  CROW_ROUTE(app_, "/chats/<int>/members")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, int chat_id) {
            PROFILE_SCOPE("/chats/<int>/members");
            controller_->getAllChatMembers(req, res, chat_id);
            LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
          });
}
