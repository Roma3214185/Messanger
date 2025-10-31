#include "controller.h"

#include <jwt-cpp/jwt.h>

#include <nlohmann/json.hpp>
#include <utility>

#include "Debug_profiling.h"
#include "MessageManager/MessageManager.h"
#include "RabbitMQClient/rabbitmqclient.h"

namespace {

inline crow::json::wvalue to_crow_json(const Message& message) {
  crow::json::wvalue json_message;
  LOG_INFO(
      "[Message] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' | "
      "timestamp '{}'",
      message.id, message.chat_id, message.sender_id, message.text,
      message.timestamp);
  json_message["id"] = message.id;
  json_message["chat_id"] = message.chat_id;
  json_message["sender_id"] = message.sender_id;
  json_message["text"] = message.text;
  json_message["timestamp"] = QDateTime::fromSecsSinceEpoch(message.timestamp)
                                 .toString(Qt::ISODate)
                                 .toStdString();
  json_message["local_id"] = message.local_id;
  LOG_INFO("Local_id for text {} is {}", message.text, message.local_id);
  return json_message;
}

constexpr const char* SECRET_KEY = "super_secret_key";
constexpr const char* ISSUER = "my-amazon-clone";
constexpr int kSuccessfullStatusCode = 200;
constexpr int kServerError = 500;
constexpr int kUserError = 400;

const std::string kSavingMessageStatusEvent = "save_message_status";
const std::string kSavingMessageEvent = "save_message";
const std::string kMessageSaved = "message_saved";
const std::string kExchange = "app.events";
const std::string kMessageQueue = "message_service_queue";
const std::string kMessageStatusSaved = "message_status_saved";

std::optional<int> verifyTokenAndGetUserId(const std::string& token) {
  try {
    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
                        .with_issuer(ISSUER);
    verifier.verify(decoded);

    int user_id = std::stoll(decoded.get_payload_claim("sub").as_string());
    LOG_INFO("Token is verified, id is '{}'", user_id);
    return user_id;
  } catch (const std::exception& e) {
    LOG_ERROR("Error verifying token, error: {}", e.what());
    return std::nullopt;
  } catch (...) {
    LOG_ERROR("Unknown error verifying token");
    return std::nullopt;
  }
}

Message from_crow_json(const crow::json::rvalue& json_message) {
  Message message;
  if (json_message.count("id")) {
    message.id = json_message["id"].i();
  } else {
    message.id = 0;
  }

  message.chat_id = json_message["chat_id"].i();
  message.sender_id = json_message["sender_id"].i();
  message.text = json_message["text"].s();
  message.local_id = json_message["local_id"].s();
  LOG_INFO("[Message] For text: {}, Local_id = ", message.text, message.local_id);

  if (json_message.count("timestamp")) {
    QString timestamp = QString::fromStdString(json_message["timestamp"].s());
    QDateTime datetime = QDateTime::fromString(timestamp, Qt::ISODate);
    if (!datetime.isValid()) {
      message.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    } else {
      message.timestamp = datetime.toSecsSinceEpoch();
    }
  } else {
    message.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
  }

  LOG_INFO(
      "[Message from json] id '{}' | chat_id '{}' | sender_id '{}' | text '{}' "
      "| timestamp '{}'",
      message.id, message.chat_id, message.sender_id, message.text,
      message.timestamp);
  return message;
}

}  // namespace

Controller::Controller(crow::SimpleApp& app, RabbitMQClient* mq_client,
                       MessageManager* manager)
    : app_(app), manager_(manager), mq_client_(mq_client) {
  subscribeSaveMessage();
  subscribeSaveMessageStatus();
}

void Controller::handleSaveMessage(const std::string& payload) {
  nlohmann::json parsed;
  LOG_INFO("Save message hanling {}", payload);
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
    if (!ok)
      LOG_ERROR("Error saving message id {}", msg.id);
    else
      mq_client_->publish(kExchange, kMessageSaved, to_json(msg).dump());
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
    if (!ok)
      LOG_ERROR("Error saving message_status id {}", status.id);
    else
      mq_client_->publish(kExchange, kMessageStatusSaved,
                          to_json(status).dump());
  });
}

void Controller::handleRoutes() { handleGetMessagesFromChat(); }

void Controller::handleGetMessagesFromChat() {
  CROW_ROUTE(app_, "/messages/<int>")
      .methods(
          crow::HTTPMethod::GET)([&](const crow::request& req, int chat_id) {
        PROFILE_SCOPE("/messages/id");
        std::string token = getToken(req);
        std::optional<int> current_user_id = verifyTokenAndGetUserId(token);

        if (!current_user_id) {  // TODO(roma): make check if u have acess to
                                 // ges these messagess
          LOG_ERROR("Can't verify token");
          return crow::response(kUserError);
        }

        int limit = req.url_params.get("limit")
                        ? std::stoi(req.url_params.get("limit"))
                        : INT_MAX;  // TODO(roma): beforeId -> before_id in
                                    // Frontend/src/Model
        int before_id = req.url_params.get("beforeId")
                            ? std::stoi(req.url_params.get("beforeId"))
                            : 0;

        LOG_INFO("For id '{}' limit is '{}' and beforeId is '{}'", chat_id,
                 limit, before_id);

        auto messages = manager_->getChatMessages(chat_id, limit, before_id);

        LOG_INFO("For chat '{}' finded '{}' messages", chat_id,
                 messages.size());
        crow::json::wvalue res = crow::json::wvalue::list();
        int i = 0;
        for (const auto& msg : messages) {
          std::optional<MessageStatus> status_mine_message =
              manager_->getMessageStatus(msg.id, *current_user_id);
          LOG_INFO("Get message status for '{}' and receiver_id '{}'", msg.id,
                   *current_user_id);
          if (!status_mine_message) {
            LOG_ERROR("status_mine_message = false");  // if u delete message
                                                       // for yourself
            continue;
          }
          auto jsonObject = to_crow_json(msg);
          jsonObject["readed_by_me"] = status_mine_message->is_read;

          res[i++] = std::move(jsonObject);
        }

        return crow::response(kSuccessfullStatusCode, res);
      });
}

std::string Controller::getToken(const crow::request& req) {
  return req.get_header_value("Authorization");
}
