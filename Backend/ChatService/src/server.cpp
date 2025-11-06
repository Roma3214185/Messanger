#include "server.h"

#include "controller.h"
#include "Debug_profiling.h"

Server::Server(const int port, ChatManager* manager) : port_(port), manager_(manager) {
  controller_ = std::make_unique<Controller>(app_, manager_);
  initRoutes();
}

void Server::initRoutes() {
  handleCreatingPrivateChat();
  handleGetAllChats();
  handleGetAllChatsById();
  handleGetAllChatsMembers();
}

void Server::run() {
  LOG_INFO("Chat microservice is running on port {}", port_);
  app_.port(port_).multithreaded().run();
}

void Server::handleCreatingPrivateChat() {
  CROW_ROUTE(app_, "/chats/private")
  .methods(crow::HTTPMethod::POST)
      ([&](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("/auth/me");
        controller_->createPrivateChat(req, res);
        LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}

void Server::handleGetAllChats() {
  CROW_ROUTE(app_, "/chats")
  .methods(crow::HTTPMethod::GET)([&](const crow::request& req, crow::response& res) {
    PROFILE_SCOPE("/chats");
    controller_->getAllChats(req, res);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
  });
}

void Server::handleGetAllChatsById() {
  CROW_ROUTE(app_, "/chats/<int>")
  .methods(crow::HTTPMethod::GET)([&](const crow::request& req, crow::response& res, int chat_id) {
    PROFILE_SCOPE("/chats/<int>");
    controller_->getAllChatsById(req, res, chat_id);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
  });
}

void Server::handleGetAllChatsMembers() {
  CROW_ROUTE(app_, "/chats/<int>/members")
  .methods(crow::HTTPMethod::GET)([&](const crow::request& req, crow::response& res, int chat_id) {
    PROFILE_SCOPE("/chats/<int>/members");
    controller_->getAllChatMembers(req, res, chat_id);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
  });
}
