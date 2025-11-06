#include "controller.h"

#include <jwt-cpp/jwt.h>

#include <nlohmann/json.hpp>
#include <utility>

#include "Debug_profiling.h"
#include "MessageManager/MessageManager.h"
#include "RabbitMQClient/rabbitmqclient.h"
#include "headers/JwtUtils.h"

namespace {

constexpr int kSuccessfullStatusCode = 200;
constexpr int kServerError = 500;
constexpr int kUserError = 400;

const std::string kSavingMessageStatusEvent = "save_message_status";
const std::string kSavingMessageEvent = "save_message";
const std::string kMessageSaved = "message_saved";
const std::string kExchange = "app.events";
const std::string kMessageQueue = "message_service_queue";
const std::string kMessageStatusSaved = "message_status_saved";

void sendResponse(crow::response& res, int code, const std::string& text) {
  res.code = code;
  res.write(text);
  res.end();
}

std::string extractToken(const crow::request& req) {
  return req.get_header_value("Authorization");
}

int getLimit(const crow::request& req) {
  return req.url_params.get("limit")
    ? std::stoi(req.url_params.get("limit"))
    : INT_MAX;
}

int getBeforeId(const crow::request& req) {
  return req.url_params.get("before_id")
    ? std::stoi(req.url_params.get("before_id"))
    : 0;
}

}  // namespace

Controller::Controller(crow::SimpleApp& app, RabbitMQClient* mq_client,
                       MessageManager* manager)
    : app_(app), manager_(manager), mq_client_(mq_client) {
  subscribeSaveMessage();
  subscribeSaveMessageStatus();
}

void Controller::handleSaveMessage(const std::string& payload) {
  LOG_INFO("Save message hanling {}", payload);
  nlohmann::json parsed;
  try {
    parsed = nlohmann::json::parse(payload);
  } catch (...) {
    LOG_ERROR("Failed to parse message payload");
    return;
  }

  Message msg;
  from_json(parsed, msg);

  LOG_INFO("Get message to save with id {} and text {}", msg.id, msg.text);

  pool_.enqueue([this, msg]() mutable {
    bool ok = manager_->saveMessage(msg);
    if (!ok) {
      LOG_ERROR("Error saving message id {}", msg.id);
    } else {
      mq_client_->publish(kExchange, kMessageSaved, to_json(msg).dump());
    }
  });
}

void Controller::subscribeSaveMessage() {
  mq_client_->subscribe(
      kMessageQueue, kExchange, kSavingMessageEvent,
      [this](const std::string& event, const std::string& payload) {
        handleSaveMessage(payload);
      },
      "topic");
}

void Controller::subscribeSaveMessageStatus() {
  mq_client_->subscribe(
      kMessageQueue, kExchange, kSavingMessageStatusEvent,
      [this](const std::string& event, const std::string& payload) {
        LOG_INFO("Save message_status hanling {}", payload);
        handleSaveMessageStatus(payload);
      },
      "topic");
}

void Controller::handleSaveMessageStatus(const std::string& payload) {
  nlohmann::json parsed;

  try {
    parsed = nlohmann::json::parse(payload);
  } catch (...) {
    LOG_ERROR("Failed to parse message_status payload");
    return;
  }

  MessageStatus status;
  from_json(parsed, status);

  pool_.enqueue([this, status]() mutable {
    bool ok = manager_->saveMessageStatus(status);
    if (!ok) {
      LOG_ERROR("Error saving message_status id {}", status.id);
    } else {
      mq_client_->publish(kExchange, kMessageStatusSaved,
                          to_json(status).dump());
    }
  });
}

void Controller::getMessagesFromChat(const crow::request& req, int chat_id, crow::response& responce) {
  std::string token = extractToken(req);
  std::optional<int> current_user_id = JwtUtils::verifyTokenAndGetUserId(token);

  if (!current_user_id) {  // TODO(roma): make check if u have acess to
                           // ges these messagess
    sendResponse(responce, kUserError, "Invalid or expired token");
    return;
  }

  int limit = getLimit(req);
  int before_id = getBeforeId(req);

  LOG_INFO("For id '{}' limit is '{}' and beforeId is '{}'", chat_id,
           limit, before_id);

  auto messages = manager_->getChatMessages(chat_id, limit, before_id);

  LOG_INFO("For chat '{}' finded '{}' messages", chat_id,
           messages.size());


  crow::json::wvalue res = formMessageListJson(messages, *current_user_id);
  sendResponse(responce, kSuccessfullStatusCode, res.dump());
}

crow::json::wvalue Controller::formMessageListJson(const std::vector<Message>& messages, int current_user_id) {
  crow::json::wvalue res = crow::json::wvalue::list();
  int i = 0;
  for (const auto& msg : messages) {
    std::optional<MessageStatus> status_mine_message =
        manager_->getMessageStatus(msg.id, current_user_id);
    LOG_INFO("Get message status for '{}' and receiver_id '{}'", msg.id,
             current_user_id);
    if (!status_mine_message) {
      LOG_ERROR("status_mine_message = false");  // if u delete message
          // for yourself
      continue;
    }
    auto jsonObject = to_crow_json(msg);
    jsonObject["readed_by_me"] = status_mine_message->is_read;

    res[i++] = std::move(jsonObject);
  }
  return res;
}
