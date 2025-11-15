#include "messageservice/controller.h"

#include <jwt-cpp/jwt.h>

#include <nlohmann/json.hpp>
#include <utility>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "messageservice/managers/JwtUtils.h"
#include "messageservice/managers/MessageManager.h"
#include "interfaces/IConfigProvider.h"
#include "interfaces/IRabitMQClient.h"

namespace {

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

template<typename T>
std::optional<T> parsePayload(const std::string& payload) {
  nlohmann::json parsed;
  try {
    parsed = nlohmann::json::parse(payload);
  } catch (...) {
    LOG_ERROR("Failed to parse message payload");
    return std::nullopt;
  }
  return parsed.get<T>();
}

}  // namespace

Controller::Controller(IRabitMQClient* mq_client,
                       MessageManager* manager, IConfigProvider* provider)
    : manager_(manager), mq_client_(mq_client), provider_(provider) {
  subscribeSaveMessage();
  subscribeSaveMessageStatus();
}

void Controller::handleSaveMessage(const std::string& payload) {
  std::optional<Message> msg  = parsePayload<Message>(payload); //TODO: alias
  if(msg == std::nullopt) return;

  auto message = *msg;
  LOG_INFO("Get message to save with id {} and text {}", message.id, message.text);

  pool_.enqueue([this, message]() mutable {
    bool ok = manager_->saveMessage(message);
    if (!ok) {
      LOG_ERROR("Error saving message id {}", message.id);
      return;
    }

    PublishRequest request;
    request.exchange = provider_->routes().exchange;
    request.message = provider_->routes().messageSaved;
    request.message = nlohmann::json(message).dump(); // can be error excahnge type

    mq_client_->publish(request);
    // TODO: kMessageSaved
  });
}

void Controller::subscribeSaveMessage() {
  SubscribeRequest request;
  request.queue = provider_->routes().messageQueue;
  request.exchange = provider_->routes().exchange;
  request.routingKey = provider_->routes().saveMessage;

  mq_client_->subscribe(request,
      [this](const std::string& event, const std::string& payload) { handleSaveMessage(payload); });
}

void Controller::subscribeSaveMessageStatus() {
  SubscribeRequest request;
  request.queue = provider_->routes().messageQueue;
  request.exchange = provider_->routes().exchange;
  request.routingKey = provider_->routes().saveMessageStatus;
  mq_client_->subscribe(request,
      [this](const std::string& event, const std::string& payload) {
        handleSaveMessageStatus(payload);
      });
}

void Controller::handleSaveMessageStatus(const std::string& payload) {
  std::optional<MessageStatus> message_status = parsePayload<MessageStatus>(payload);
  if(message_status == std::nullopt) return;

  auto status = *message_status;

  pool_.enqueue([this, status]() mutable {
    if (!manager_->saveMessageStatus(status)) {
      LOG_ERROR("Error saving message_status id {}", status.message_id);
      return;
    }

    PublishRequest request; // topic type was
    request.exchange = provider_->routes().exchange;
    request.routingKey = provider_->routes().messageStatusSaved;
    request.message = nlohmann::json(status).dump();

    mq_client_->publish(request);
  });
}

void Controller::getMessagesFromChat(const crow::request& req,
                                     int                  chat_id,
                                     crow::response&      responce) {
  std::string        token           = extractToken(req);
  std::optional<int> current_user_id = JwtUtils::verifyTokenAndGetUserId(token);

  if (!current_user_id) {  // TODO(roma): make check if u have acess to
                           // ges these messagess
    sendResponse(responce, provider_->statusCodes().userError, "Invalid or expired token");
    return;
  }

  int limit     = getLimit(req);
  int before_id = getBeforeId(req);

  LOG_INFO("For id '{}' limit is '{}' and beforeId is '{}'", chat_id, limit, before_id);

  auto messages = manager_->getChatMessages(chat_id, limit, before_id);

  LOG_INFO("For chat '{}' finded '{}' messages", chat_id, messages.size());

  crow::json::wvalue res = formMessageListJson(messages, *current_user_id);
  sendResponse(responce, provider_->statusCodes().success, res.dump());
}

crow::json::wvalue Controller::formMessageListJson(const std::vector<Message>& messages,
                                                   int                         current_user_id) {
  crow::json::wvalue res = crow::json::wvalue::list();
  int                i   = 0;
  for (const auto& msg : messages) {
    std::optional<MessageStatus> status_mine_message =
        manager_->getMessageStatus(msg.id, current_user_id);
    LOG_INFO("Get message status for '{}' and receiver_id '{}'", msg.id, current_user_id);
    if (!status_mine_message) {
      LOG_ERROR("status_mine_message = false");  // if u delete message
                                                 // for yourself
      continue;
    }
    auto jsonObject            = to_crow_json(msg);
    jsonObject["readed_by_me"] = status_mine_message->is_read;

    res[i++] = std::move(jsonObject);
  }
  return res;
}
