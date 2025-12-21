#include "chatservice/chatserver.h"

#include "chatservice/chatcontroller.h"
#include "Debug_profiling.h"
#include "interfaces/IApp.h"

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

void ChatServer::handleGetChat() {
  CROW_ROUTE(app_, "/chats/<string>")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, const std::string& chat_id_str) {
            PROFILE_SCOPE("/chats/id");
            LOG_INFO("chat_id_str = {}", chat_id_str);
            long long chat_id;
            try {
              chat_id = std::stoll(chat_id_str);
            } catch(...) {
              LOG_ERROR("Can't get stoll in handleGetChat");
              res.code = 400;
              res.body = "Invalid id";
              res.end();
              return;
            }
            LOG_INFO("Chat id for str {} is {}", chat_id_str, chat_id);

            controller_->getChat(req, res, chat_id);
            LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
          });
}

void ChatServer::handleGetAllChatsMembers() {
  CROW_ROUTE(app_, "/chats/<string>/members")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, const std::string& chat_id_str) {
            PROFILE_SCOPE("/chats/id/members");
            LOG_INFO("chat_id_str = {}", chat_id_str);
            long long chat_id;
            try {
              chat_id = std::stoll(chat_id_str);
            } catch(...) {
              LOG_ERROR("Can't get stoll in handleGetChat");
              res.code = 400;
              res.body = "Invalid id";
              res.end();
              return;
            }
            LOG_INFO("Chat id for str {} is {}", chat_id_str, chat_id);

            controller_->getAllChatMembers(req, res, chat_id);
            LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
          });
}
