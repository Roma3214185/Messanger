#include "config/Routes.h"
#include "interfaces/IRabitMQClient.h"
#include "notificationservice/ISubscriber.h"
#include "notificationservice/managers/NotificationOrchestrator.h"

void RabbitNotificationSubscriber::subscribeMessageDeleted() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageDeleted;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageDeleted;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    notification_orchestrator_->onMessageDeleted(payload);
  });
}

void RabbitNotificationSubscriber::subscribeMessageStatusSaved() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageSavedQueue;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageStatusSaved;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    notification_orchestrator_->onMessageStatusSaved(payload);
  });
}

void RabbitNotificationSubscriber::subscribeMessageSaved() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageSavedQueue;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageSaved;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    notification_orchestrator_->onMessageSaved(payload);
  });
}

RabbitNotificationSubscriber::RabbitNotificationSubscriber(IEventSubscriber *mq_client,
                                                           NotificationOrchestrator *notification_orchestrator)
    : mq_client_(mq_client), notification_orchestrator_(notification_orchestrator) {}

void RabbitNotificationSubscriber::subscribeAll() {
  subscribeMessageSaved();
  subscribeMessageDeleted();
  subscribeMessageStatusSaved();
  subscribeMessageReactionDeleted();
  subscribeMessageReactionSaved();
}

void RabbitNotificationSubscriber::subscribeMessageReactionDeleted() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageReactionDeleted;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageReactionDeleted;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    notification_orchestrator_->onMessageReactionDeleted(payload);
  });
}

void RabbitNotificationSubscriber::subscribeMessageReactionSaved() {
  SubscribeRequest request;
  request.queue = Config::Routes::messageReactionSaved;
  request.exchange = Config::Routes::exchange;
  request.routing_key = Config::Routes::messageReactionSaved;
  request.exchange_type = Config::Routes::exchangeType;

  mq_client_->subscribe(request, [this](const std::string &event, const std::string &payload) {
    notification_orchestrator_->onMessageReactionSaved(payload);
  });
}
