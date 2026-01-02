#include "messageservice/controller.h"

#include <nlohmann/json.hpp>
#include <utility>

#include "Debug_profiling.h"
#include "RabbitMQClient.h"
#include "messageservice/managers/MessageManager.h"
#include "interfaces/IConfigProvider.h"
#include "interfaces/IRabitMQClient.h"
#include "interfaces/IThreadPool.h"
#include "messageservice/managers/JwtUtils.h"
#include "entities/Message.h"

namespace {

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

std::optional<long long> getIdFromStr(const std::string& str) {
  try {
    return std::stoll(str);
  } catch(...) {
    LOG_ERROR("Error while get stool of {}", str);
    return std::nullopt;
  }
}

std::string formErrorResponce(const std::string& error_text){
  nlohmann::json json;
  json["error"] = error_text;
  return json.dump();
}

}  // namespace

Controller::Controller(IRabitMQClient* mq_client,
                       MessageManager* manager, IThreadPool* pool, IConfigProvider* provider)
    : manager_(manager), mq_client_(mq_client),  pool_(pool), provider_(provider) {
  subscribeToSaveMessage();
  subscribeToSaveMessageStatus();
}

void Controller::handleSaveMessage(const std::string& payload) {
  std::optional<Message> msg  = parsePayload<Message>(payload); //TODO: alias
  if(msg == std::nullopt) return;

  auto message = *msg;
  LOG_INFO("Get message to save with id {} and text {}", message.id, message.text);

  pool_->enqueue([this, message]() mutable {
    if (!manager_->saveMessage(message)) {
      LOG_ERROR("Error saving message id {}", message.id);
      return;
    }

    PublishRequest request;
    request.exchange = provider_->routes().exchange;
    request.routing_key = provider_->routes().messageSaved;
    request.message = nlohmann::json(message).dump(); // can be error excahnge type
    request.exchange_type = provider_->routes().exchangeType;

    mq_client_->publish(request);
    // TODO: kMessageSaved
  });
}

void Controller::subscribeToSaveMessage() {
  SubscribeRequest request;
  request.queue = provider_->routes().saveMessageQueue;
  request.exchange = provider_->routes().exchange;
  request.routing_key = provider_->routes().saveMessage;
  request.exchange_type = provider_->routes().exchangeType;
  mq_client_->subscribe(request,
                        [this](const std::string& event, const std::string& payload) {
                          LOG_INFO("Getted event in subscribeToSaveMessage: {} and payload {}", event, payload);
                          if(event == provider_->routes().saveMessage) handleSaveMessage(payload);
                        });
}

void Controller::subscribeToSaveMessageStatus() {
  SubscribeRequest request;
  request.queue = provider_->routes().saveMessageStatusQueue;
  request.exchange = provider_->routes().exchange;
  request.routing_key = provider_->routes().saveMessageStatus;
  request.exchange_type = provider_->routes().exchangeType;
  mq_client_->subscribe(request,
      [this](const std::string& event, const std::string& payload) {
        LOG_INFO("Getted event in subscribeToSaveMessageStatus: {} and payload {}", event, payload);
        if(event == provider_->routes().saveMessageStatus) handleSaveMessageStatus(payload);
      });
}

void Controller::handleSaveMessageStatus(const std::string& payload) {
  std::optional<MessageStatus> message_status = parsePayload<MessageStatus>(payload);
  if(message_status == std::nullopt) return;

  auto status = *message_status;

  pool_->enqueue([this, status]() mutable {
    if (!manager_->saveMessageStatus(status)) {
      LOG_ERROR("Error saving message_status id {}", status.message_id);
      return;
    }

    PublishRequest request{
      .exchange = provider_->routes().exchange,
      .routing_key = provider_->routes().messageStatusSaved,
      .message = nlohmann::json(status).dump(),
      .exchange_type = provider_->routes().exchangeType
    };

    mq_client_->publish(request);
  });
}

std::vector<Message> Controller::getMessages(const GetMessagePack& pack) {
  return manager_->getChatMessages(pack);
}

std::vector<MessageStatus> Controller::getMessagesStatus(const std::vector<Message>& messages, long long receiver_id) {
  return manager_->getMessagesStatus(messages, receiver_id);
}

std::optional<long long> Controller::getUserIdFromToken(const std::string& token) {
  return JwtUtils::verifyTokenAndGetUserId(token);
}

Responce Controller::updateMessage(const RequestDTO& request_pack, const std::string& message_id_str) {
  auto id_opt = getIdFromStr(message_id_str);
  if(!id_opt){
    return std::make_pair(provider_->statusCodes().badRequest, formErrorResponce("Invalid id"));
  }

  long long message_id = *id_opt;
  LOG_INFO("Id of message to update = {}", message_id);

  std::optional<long long> optional_user_id = getUserIdFromToken(request_pack.token);
  if(!optional_user_id){
    return std::make_pair(provider_->statusCodes().badRequest, formErrorResponce(provider_->issueMessages().invalidToken));
  }

  long long current_user_id = *optional_user_id;
  LOG_INFO("Current id = {}", current_user_id);

  //check if u have access to update this message (update only curr user)
  std::optional<Message> message_to_update = manager_->getMessage(message_id);
  if(!message_to_update) {
    return std::make_pair(provider_->statusCodes().notFound, formErrorResponce("Message to update not found"));
  }

  if(message_to_update->sender_id != current_user_id) {
    return std::make_pair(provider_->statusCodes().conflict, formErrorResponce("U have no permission to update this message"));
  }

  std::optional<Message> updated_message = parsePayload<Message>(request_pack.body);
  if(!updated_message) {
    return std::make_pair(provider_->statusCodes().badRequest, formErrorResponce("Invalid data"));
  }

  //todo: check invariants of updated_message;
  if(!manager_->saveMessage(*updated_message)) {
    return std::make_pair(provider_->statusCodes().serverError, formErrorResponce("Error while saving"));
  }

  PublishRequest request;
  request.exchange = provider_->routes().exchange;
  request.routing_key = provider_->routes().messageSaved;
  request.message = nlohmann::json(*updated_message).dump();
  request.exchange_type = provider_->routes().exchangeType;

  mq_client_->publish(request);
  return std::make_pair(200, nlohmann::json(*updated_message).dump());
}

Responce Controller::deleteMessage(const RequestDTO& request_pack, const std::string& message_id_str) {
  auto id_opt = getIdFromStr(message_id_str);
  if(!id_opt){
    return std::make_pair(provider_->statusCodes().badRequest, formErrorResponce("Invalid id"));
  }

  long long message_id = *id_opt;

  std::optional<long long> optional_user_id = getUserIdFromToken(request_pack.token);
  if(!optional_user_id){
    return std::make_pair(provider_->statusCodes().badRequest, formErrorResponce(provider_->issueMessages().invalidToken));
  }

  long long current_user_id = *optional_user_id;
  LOG_INFO("Current id = {}", current_user_id);

  std::optional<Message> message_to_delete = manager_->getMessage(message_id);
  if(!message_to_delete) {
    return std::make_pair(provider_->statusCodes().badRequest, formErrorResponce("Invalid data"));
  }

  if(message_to_delete->sender_id != current_user_id) { //in future permission will be in admins of the group chat
    return std::make_pair(provider_->statusCodes().conflict, formErrorResponce("U have no permission to update this message"));
  }

  if(!manager_->deleteMessage(*message_to_delete)) {
    return std::make_pair(provider_->statusCodes().serverError, formErrorResponce("Error while saving"));
  }

  PublishRequest request;
  request.exchange = provider_->routes().exchange;
  request.routing_key = provider_->routes().messageDeleted;
  request.message = nlohmann::json(*message_to_delete).dump();
  request.exchange_type = provider_->routes().exchangeType;

  mq_client_->publish(request);

  return std::make_pair(200, nlohmann::json(*message_to_delete).dump());
}

std::vector<MessageStatus> Controller::getReadedMessageStatuses(long long message_id) {
  return manager_->getReadedMessageStatuses(message_id);
}

Responce Controller::getMessageById(long long message_id) {
  LOG_INFO("getMessageById {}", message_id);
  auto message_opt = manager_->getMessage(message_id);
  if(!message_opt) return std::make_pair(404, formErrorResponce("Not found"));
  return std::make_pair(200, nlohmann::json(*message_opt).dump());

}
