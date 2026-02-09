#include "messageservice/QueueSubscriber.h"
#include "interfaces/IRabitMQClient.h"
#include "Debug_profiling.h"
#include "config/Routes.h"

QueueSubscriber::QueueSubscriber(IEventSubscriber *mq_client) : mq_client_(mq_client) {}

void QueueSubscriber::subscribeToSaveMessage(Callback callback) {
    SubscribeRequest request;
    request.queue = Config::Routes::saveMessageQueue;
    request.exchange = Config::Routes::exchange;
    request.routing_key = Config::Routes::saveMessage;
    request.exchange_type = Config::Routes::exchangeType;
    mq_client_->subscribe(request, [callback](const std::string &event, const std::string &payload) {
        LOG_INFO("Getted event in subscribeToSaveMessage: {} and payload {}", event, payload);
        if (event == Config::Routes::saveMessage) callback(payload);
    });
}

void QueueSubscriber::subscribeToSaveMessageStatus(Callback callback) {
    SubscribeRequest request;
    request.queue = Config::Routes::saveMessageStatusQueue;
    request.exchange = Config::Routes::exchange;
    request.routing_key = Config::Routes::saveMessageStatus;
    request.exchange_type = Config::Routes::exchangeType;
    mq_client_->subscribe(request, [callback](const std::string &event, const std::string &payload) {
        LOG_INFO("Getted event in subscribeToSaveMessageStatus: {} and payload {}", event, payload);
        if (event == Config::Routes::saveMessageStatus) callback(payload);
    });
}

void QueueSubscriber::subscribeToSaveMessageReaction(Callback callback) {
    SubscribeRequest request;
    request.queue = Config::Routes::saveReaction;
    request.exchange = Config::Routes::exchange;
    request.routing_key = Config::Routes::saveReaction;
    request.exchange_type = Config::Routes::exchangeType;
    mq_client_->subscribe(request, [callback](const std::string &event, const std::string &payload) {
        LOG_INFO("Getted event in onSaveMessageReaction: {} and payload {}", event, payload);
        if (event == Config::Routes::saveReaction) callback(payload);
    });
}

void QueueSubscriber::subscribeToDeleteMessageReaction(Callback callback) {
    SubscribeRequest request;
    request.queue = Config::Routes::deleteReaction;
    request.exchange = Config::Routes::exchange;
    request.routing_key = Config::Routes::deleteReaction;
    request.exchange_type = Config::Routes::exchangeType;
    mq_client_->subscribe(request, [callback](const std::string &event, const std::string &payload) {
        LOG_INFO("Getted event in onDeleteMessageReaction: {} and payload {}", event, payload);
        if (event == Config::Routes::deleteReaction) callback(payload);
    });
}
