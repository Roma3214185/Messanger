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
#include "interfaces/IThreadPool.h"

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
    bool ok = manager_->saveMessage(message);
    if (!ok) {
      LOG_ERROR("Error saving message id {}", message.id);
      return;
    }

    PublishRequest request;
    request.exchange = provider_->routes().exchange;
    request.routingKey = provider_->routes().messageSaved;
    request.message = nlohmann::json(message).dump(); // can be error excahnge type
    request.exchangeType = provider_->routes().exchangeType;

    mq_client_->publish(request);
    // TODO: kMessageSaved
  });
}

void Controller::subscribeToSaveMessage() {
  SubscribeRequest request;
  request.queue = provider_->routes().saveMessageQueue;
  request.exchange = provider_->routes().exchange;
  request.routingKey = provider_->routes().saveMessage;
  request.exchangeType = provider_->routes().exchangeType;
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
  request.routingKey = provider_->routes().saveMessageStatus;
  request.exchangeType = provider_->routes().exchangeType;
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

    PublishRequest request;
    request.exchange = provider_->routes().exchange;
    request.routingKey = provider_->routes().messageStatusSaved;
    request.message = nlohmann::json(status).dump();
    request.exchangeType = provider_->routes().exchangeType;

    mq_client_->publish(request);
  });
}

std::vector<Message> Controller::getMessages(const GetMessagePack& pack) {
  return manager_->getChatMessages(pack);
}

std::vector<MessageStatus> Controller::getMessagesStatus(const std::vector<Message>& messages, int receiver_id) {
  return manager_->getMessagesStatus(messages, receiver_id);
}
