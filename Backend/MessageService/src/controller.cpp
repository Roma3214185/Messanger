#include "messageservice/controller.h"

#include <nlohmann/json.hpp>
#include <utility>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "config/Routes.h"
#include "config/codes.h"
#include "entities/Message.h"
#include "entities/RequestDTO.h"
#include "entities/UserMessage.h"
#include "interfaces/IRabitMQClient.h"
#include "interfaces/IThreadPool.h"
#include "messageservice/dto/GetMessagePack.h"
#include "messageservice/managers/JwtUtils.h"
#include "messageservice/managers/MessageManager.h"

namespace {

template <typename T>
std::optional<T> parsePayload(const std::string &payload) {
  nlohmann::json parsed;
  try {
    parsed = nlohmann::json::parse(payload);
  } catch (...) {
    LOG_ERROR("Failed to parse message payload");
    return std::nullopt;
  }
  return parsed.get<T>();
}

std::optional<long long> getIdFromStr(const std::string &str) {
  try {
    return std::stoll(str);
  } catch (...) {
    LOG_ERROR("Error while get stoll of {}", str);
    return std::nullopt;
  }
}

int getLimit(const RequestDTO &req) {
  auto it = req.url_params.find("limit");
  return it != req.url_params.end() ? std::stoi(it->second) : INT_MAX;
}

int getBeforeId(const RequestDTO &req) {
  auto it = req.url_params.find("before_id");
  return it != req.url_params.end() ? std::stoi(it->second) : 0;
}

nlohmann::json formMessageListJson(const std::vector<UserMessage> &messages) {
  nlohmann::json res;
  int i = 0;
  for (const auto &msg : messages) {
    res[i++] = nlohmann::json(msg);
  }

  return res;
}

crow::json::wvalue formMessageListJson(const std::vector<Message> &messages,  // todo: in jsonservice
                                       const std::vector<MessageStatus> &messages_status) {
  crow::json::wvalue res = crow::json::wvalue::list();
  if (messages.size() != messages_status.size()) {
    LOG_ERROR("formMessageListJson different size of input {} vs {}", messages.size(), messages_status.size());
    return res;
  }
  int i = 0;
  for (const auto &msg : messages) {
    auto json_object = utils::entities::to_crow_json(msg);  // todo: make all using nlohmann::json
    json_object["readed_by_me"] = messages_status[i].is_read;
    res[i++] = std::move(json_object);
  }

  return res;
}

std::vector<MessageStatus> fetchReaded(const std::vector<MessageStatus> &messages_status) {
  std::vector<MessageStatus> ans;
  for (auto &msg : messages_status) {
    if (msg.is_read) ans.emplace_back(msg);
  }

  return ans;
}

}  // namespace

Controller::Controller(IRabitMQClient *mq_client, MessageManager *manager, IThreadPool *pool)
    : manager_(manager), mq_client_(mq_client), pool_(pool) {
  subscribeToSaveMessage();
  subscribeToSaveMessageStatus();
}

void Controller::handleSaveMessage(const std::string &payload) {
  std::optional<Message> msg = parsePayload<Message>(payload);  // TODO: alias
  if (msg == std::nullopt) return;                              // DBC_REQUIRE

  auto message = *msg;
  LOG_INFO("Get message to save with id {} and text {}", message.id, message.text);

  pool_->enqueue([this, message]() mutable {
    if (!manager_->saveMessage(message)) {
      LOG_ERROR("Error saving message id {}", message.id);
      return;
    }

    PublishRequest request;
    request.exchange = Config::Routes::exchange;
    request.routing_key = Config::Routes::messageSaved;
    request.message = nlohmann::json(message).dump();  // can be error excahnge type
    request.exchange_type = Config::Routes::exchangeType;

    mq_client_->publish(request);
    // TODO: kMessageSaved
  });
}

void Controller::subscribeToSaveMessage() {
  SubscribeRequest request;
  request.queue = Config::Routes::saveMessageQueue;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::saveMessage;
  request.exchange_type = Config::Routes::exchangeType;
  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO("Getted event in subscribeToSaveMessage: {} and payload {}", event, payload);
    if (event == Config::Routes::saveMessage) handleSaveMessage(payload);
  });
}

void Controller::subscribeToSaveMessageStatus() {
  SubscribeRequest request;
  request.queue = Config::Routes::saveMessageStatusQueue;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::saveMessageStatus;
  request.exchange_type = Config::Routes::exchangeType;
  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    LOG_INFO("Getted event in subscribeToSaveMessageStatus: {} and payload {}", event, payload);
    if (event == Config::Routes::saveMessageStatus) handleSaveMessageStatus(payload);
  });
}

void Controller::handleSaveMessageStatus(const std::string &payload) {
  std::optional<MessageStatus> message_status = parsePayload<MessageStatus>(payload);
  if (message_status == std::nullopt) return;

  auto status = *message_status;

  pool_->enqueue([this, status]() mutable {
    if (!manager_->saveMessageStatus(status)) {
      LOG_ERROR("Error saving message_status id {}", status.message_id);
      return;
    }

    PublishRequest request{.exchange = Config::Routes::exchange,
                           .routing_key = Config::Routes::messageStatusSaved,
                           .message = nlohmann::json(status).dump(),
                           .exchange_type = Config::Routes::exchangeType};

    mq_client_->publish(request);
  });
}

std::vector<Message> Controller::getMessages(const GetMessagePack &pack) { return manager_->getChatMessages(pack); }

std::vector<MessageStatus> Controller::getMessagesStatus(const std::vector<Message> &messages, long long receiver_id) {
  return manager_->getMessagesStatus(messages, receiver_id);
}

std::optional<long long> Controller::getUserIdFromToken(const std::string &token) {
  return JwtUtils::verifyTokenAndGetUserId(token);
}

Response Controller::updateMessage(const RequestDTO &request_pack, const std::string &message_id_str) {
  auto id_opt = getIdFromStr(message_id_str);
  if (!id_opt.has_value()) {
    return std::make_pair(Config::StatusCodes::badRequest, utils::details::formError("Invalid id"));
  }

  long long message_id = *id_opt;
  LOG_INFO("Id of message to update = {}", message_id);

  std::optional<long long> optional_user_id = getUserIdFromToken(request_pack.token);
  if (!optional_user_id.has_value()) {
    return std::make_pair(Config::StatusCodes::badRequest, formErrorResponse(Config::IssueMessages::invalidToken));
  }

  long long current_user_id = *optional_user_id;
  LOG_INFO("Current id = {}", current_user_id);

  // check if u have access to update this message (update only curr user)
  std::optional<Message> message_to_update = manager_->getMessage(message_id);
  if (!message_to_update) {
    return std::make_pair(Config::StatusCodes::notFound, utils::details::formError("Message to update not found"));
  }

  if (message_to_update->sender_id != current_user_id) {
    return std::make_pair(Config::StatusCodes::conflict,
                          utils::details::formError("U have no permission to update this message"));
  }

  std::optional<Message> updated_message = parsePayload<Message>(request_pack.body);
  if (!updated_message) {
    return std::make_pair(Config::StatusCodes::badRequest, utils::details::formError("Invalid data"));
  }

  // todo: check invariants of updated_message;
  if (!manager_->saveMessage(*updated_message)) {
    return std::make_pair(Config::StatusCodes::serverError, utils::details::formError("Error while saving"));
  }

  PublishRequest request;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageSaved;
  request.message = nlohmann::json(*updated_message).dump();
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->publish(request);
  return std::make_pair(200, nlohmann::json(*updated_message).dump());
}

Response Controller::deleteMessage(const RequestDTO &request_pack, const std::string &message_id_str) {
  auto id_opt = getIdFromStr(message_id_str);
  if (!id_opt) {
    return std::make_pair(Config::StatusCodes::badRequest, utils::details::formError("Invalid id"));
  }

  long long message_id = *id_opt;

  std::optional<long long> optional_user_id = getUserIdFromToken(request_pack.token);
  if (!optional_user_id.has_value()) {
    return std::make_pair(Config::StatusCodes::badRequest, formErrorResponse(Config::IssueMessages::invalidToken));
  }

  long long current_user_id = *optional_user_id;
  LOG_INFO("Current id = {}", current_user_id);

  std::optional<Message> message_to_delete = manager_->getMessage(message_id);
  if (!message_to_delete) {
    return std::make_pair(Config::StatusCodes::notFound, utils::details::formError("Message to delete not found"));
  }

  if (message_to_delete->sender_id != current_user_id) {  // in future permission will be in admins of the group
                                                          // chat
    return std::make_pair(Config::StatusCodes::conflict,
                          utils::details::formError("U have no permission to update this message"));
  }

  if (!manager_->deleteMessage(*message_to_delete)) {
    return std::make_pair(Config::StatusCodes::serverError, utils::details::formError("Error while saving"));
  }

  PublishRequest request;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageDeleted;
  request.message = nlohmann::json(*message_to_delete).dump();
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->publish(request);

  return std::make_pair(200, nlohmann::json(*message_to_delete).dump());
}

std::vector<MessageStatus> Controller::getReadedMessageStatuses(long long message_id) {
  return manager_->getReadedMessageStatuses(message_id);
}

Response Controller::getMessageById(const std::string &message_id_str) {
  LOG_INFO("getMessageById {}", message_id_str);

  std::optional<long long> message_id = getIdFromStr(message_id_str);
  if (!message_id.has_value()) return std::make_pair(400, utils::details::formError("Invalid message_id"));
  auto message_opt = manager_->getMessage(*message_id);
  return message_opt ? std::make_pair(200, nlohmann::json(*message_opt).dump())
                     : std::make_pair(404, utils::details::formError("Not found"));
}

Response Controller::getMessagesFromChat(const RequestDTO &request_pack, const std::string &chat_id_str) {
  std::optional<long long> user_id = getUserIdFromToken(request_pack.token);
  if (!user_id.has_value()) {
    return std::make_pair(Config::StatusCodes::userError, formErrorResponse(Config::IssueMessages::invalidToken));
  }

  std::optional<long long> chat_id = getIdFromStr(chat_id_str);
  if (!chat_id.has_value()) {
    return std::make_pair(Config::StatusCodes::badRequest, utils::details::formError("Invalid chat_id"));
  }

  const GetMessagePack pack{.chat_id = *chat_id,
                            .limit = getLimit(request_pack),
                            .before_id = getBeforeId(request_pack),
                            .user_id = *user_id};

  auto messages = getMessages(pack);
  auto messages_status = getMessagesStatus(messages, *user_id);
  auto json_messages = formMessageListJson(messages, messages_status);
  // auto messages_status_readed = fetchReaded(messages_status);
  // UserMessage Response

  // i need to return std::vector<UserMessage>
  std::vector<UserMessage> ans;
  for (auto &message : messages) {
    std::vector<MessageStatus> message_statuses = getReadedMessageStatuses(message.id);
    UserMessage user_message;
    user_message.message = message;
    user_message.read.count = message_statuses.size();

    bool is_read_by_me = false;
    for (auto &message_status : message_statuses) {
      if (message_status.receiver_id == *user_id) {
        is_read_by_me = true;
        break;
      }
    }
    user_message.read.read_by_me = is_read_by_me;
    // todo: reactions here
  }

  return std::make_pair(Config::StatusCodes::success, json_messages.dump());
}
