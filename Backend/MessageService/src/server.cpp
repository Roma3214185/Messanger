#include "messageservice/server.h"

#include "messageservice/controller.h"
#include "interfaces/IRabitMQClient.h"
#include "messageservice/managers/JwtUtils.h"
#include "messageservice/interfaces/IController.h"
#include "messageservice/dto/GetMessagePack.h"
#include "entities/RequestDTO.h"
#include "entities/UserMessage.h"

namespace {

void sendResponse(crow::response& res, int code, const std::string& text) {
  res.code = code;
  res.write(text);
  res.end();
}

std::string extractToken(const crow::request& req) { return req.get_header_value("Authorization"); }

int getLimit(const crow::request& req) {
  return req.url_params.get("limit") != nullptr ? std::stoi(req.url_params.get("limit")) : INT_MAX;
}

int getBeforeId(const crow::request& req) {
  return req.url_params.get("before_id") != nullptr ? std::stoi(req.url_params.get("before_id")) : 0;
}

nlohmann::json formMessageListJson(const std::vector<UserMessage>& messages) {
  nlohmann::json res;
  int i   = 0;
  for (const auto& msg : messages) {
    //auto json_object =
    res[i++] = nlohmann::json(msg);
  }

  return res;
}

}  // namespace

crow::json::wvalue Server::formMessageListJson(const std::vector<Message>& messages, //todo: in jsonservice
                                       const std::vector<MessageStatus>& messages_status) {
  crow::json::wvalue res = crow::json::wvalue::list();
  if(messages.size() != messages_status.size()) {
    LOG_ERROR("formMessageListJson different size of input {} vs {}",
              messages.size(), messages_status.size());
    return res;
  }
  int i   = 0;
  for (const auto& msg : messages) {
    auto json_object            = utils::entities::to_crow_json(msg); //todo: make all using nlohmann::json
    json_object["readed_by_me"] = messages_status[i].is_read;
    res[i++] = std::move(json_object);
  }

  return res;
}

Server::Server(crow::SimpleApp& app, int port, IController* controller, IConfigProvider* provider)
    : port_(port), controller_(controller), app_(app), provider_(provider) {
  handleRoutes();
}

Server::OptionalId Server::getUserIdFromToken(const std::string& token) {
  return JwtUtils::verifyTokenAndGetUserId(token);
}

void Server::handleRoutes() {
  handleGetMessage();
  handleGetMessagesFromChat();
  handleUpdateMessage();
  handleDeleteMessage();
}

void Server::handleGetMessagesFromChat() { //todo: chat/<string>/messages
  CROW_ROUTE(app_, "/messages/<string>")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, const std::string& chat_id_str) {
            PROFILE_SCOPE();
            LOG_INFO("GEt Messages of chat");
            long long chat_id;
            try {
              chat_id = std::stoll(chat_id_str);
            } catch (...) {
              LOG_ERROR("Error whyle stoll in handleGetMessagesFromChat");
              res.code = 400;
              res.body = "Invalid user_id";
              res.end();
              return;
            }
            onGetMessagesFromChat(req, chat_id, res);
            LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
          });
}

void Server::handleGetMessage() {
  CROW_ROUTE(app_, "/message/<string>")
      .methods(crow::HTTPMethod::GET)(
          [&](const crow::request& req, crow::response& res, const std::string& message_id_str) {
            PROFILE_SCOPE();
            LOG_INFO("GEt Message by id");
            long long message_id;
            try {
              message_id = std::stoll(message_id_str);
            } catch (...) {
              LOG_ERROR("Error whyle stoll in handleGetMessage");
              res.code = 400;
              res.body = "Invalid message_id";
              res.end();
              return;
            }
            auto [code, body] = controller_->getMessageById(message_id);
            res.code = code;
            res.body = body;
            res.end();
            LOG_INFO("Response code: {} | Body: {}", code, body);
          });
}

void Server::run() {
  spdlog::info("[Message server is started on port '{}'", port_);
  app_.port(port_).multithreaded().run();
}

std::vector<MessageStatus> fetchReaded(const std::vector<MessageStatus>& messages_status) {
  std::vector<MessageStatus> ans;
  for(auto &msg: messages_status) {
    if(msg.is_read) ans.emplace_back(msg);
  }

  return ans;
}

void Server::onGetMessagesFromChat(const crow::request& req, long long chat_id, crow::response& res) {
  std::string token = extractToken(req);
  std::optional<long long> optional_user_id = getUserIdFromToken(token);
  if(!optional_user_id){
    return sendResponse(res, provider_->statusCodes().userError, provider_->issueMessages().invalidToken);
  }

  const long long user_id = *optional_user_id;

  const GetMessagePack pack {.chat_id = chat_id, .limit = getLimit(req),
                      .before_id = getBeforeId(req), .user_id = user_id };

  auto messages = controller_->getMessages(pack);
  auto messages_status = controller_->getMessagesStatus(messages, user_id);
  auto json_messages = formMessageListJson(messages, messages_status);
  //auto messages_status_readed = fetchReaded(messages_status);
  //UserMessage responce

  //i need to return std::vector<UserMessage>
  std::vector<UserMessage> ans;
  for(auto& message : messages) {
    std::vector<MessageStatus> message_statuses = controller_->getReadedMessageStatuses(message.id);
    UserMessage user_message;
    user_message.message = message;
    user_message.read.count = message_statuses.size();

    bool is_read_by_me = false;
    for(auto& message_status : message_statuses) {
      if(message_status.receiver_id == user_id) {
        is_read_by_me = true;
        break;
      }
    }
    user_message.read.read_by_me = is_read_by_me;
    //todo: reactions here
  }

  //sendResponse(res, provider_->statusCodes().success,  formMessageListJson(ans);
  sendResponse(res, provider_->statusCodes().success, json_messages.dump());
}

void Server::handleUpdateMessage(){
  CROW_ROUTE(app_, "/messages/<string>")
      .methods(crow::HTTPMethod::PUT)(
          [&](const crow::request& req, const std::string& message_id_str) {
            LOG_INFO("Get reqeust to update message with id {}", message_id_str);
            auto request_pack = utils::getDTO(req, "message/string");
            auto [code, body] = controller_->updateMessage(request_pack, message_id_str);
            LOG_INFO("Update message with id {}, code {} and body {}", message_id_str, code, body);
            return crow::response(code, body);
          });
}

void Server::handleDeleteMessage(){
  CROW_ROUTE(app_, "/messages/<string>")
  .methods(crow::HTTPMethod::DELETE)(
      [&](const crow::request& req, const std::string& message_id_str) {
        LOG_INFO("Get reqeust to delet message with id {}", message_id_str);
        auto request_pack = utils::getDTO(req, "message/string");
        auto [code, body] = controller_->deleteMessage(request_pack, message_id_str);
        LOG_INFO("Delete message with id {}, code {} and body {}", message_id_str, code, body);
        return crow::response(code, body);
      });
}
