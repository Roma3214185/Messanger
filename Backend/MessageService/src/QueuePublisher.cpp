#include "messageservice/QueuePublisher.h"

#include "entities/Message.h"
#include "interfaces/IRabitMQClient.h"
#include "config/Routes.h"
#include "entities/MessageStatus.h"
#include "entities/Reaction.h"

QueuePublisher::QueuePublisher(IEventPublisher* mq_client) : mq_client_(mq_client) {}

void QueuePublisher::messageSaved(const Message& saved_message) {
    PublishRequest request;
    request.exchange = Config::Routes::exchange;
    request.routing_key = Config::Routes::messageSaved;
    request.message = nlohmann::json(saved_message).dump();
    request.exchange_type = Config::Routes::exchangeType;
    mq_client_->publish(request);
}

void QueuePublisher::messageDeleted(const Message& deleted_message) {
    PublishRequest request;
    request.exchange = Config::Routes::exchange;
    request.routing_key = Config::Routes::messageDeleted;
    request.message = nlohmann::json(deleted_message).dump();
    request.exchange_type = Config::Routes::exchangeType;
    mq_client_->publish(request);
}

void QueuePublisher::reactionDeleted(const Reaction& deleted_reaction) {
    PublishRequest request{.exchange = Config::Routes::exchange,
                           .routing_key = Config::Routes::messageReactionDeleted,
                           .message = nlohmann::json(deleted_reaction).dump(),
                           .exchange_type = Config::Routes::exchangeType};

    mq_client_->publish(request);
}

void QueuePublisher::reactionSaved(const Reaction& saved_reaction) {
    PublishRequest request{.exchange = Config::Routes::exchange,
                           .routing_key = Config::Routes::messageReactionSaved,
                           .message = nlohmann::json(saved_reaction).dump(),
                           .exchange_type = Config::Routes::exchangeType};

    mq_client_->publish(request);
}

void QueuePublisher::messageStatusSaved(const MessageStatus& saved_message_status) {
    PublishRequest request{.exchange = Config::Routes::exchange,
                           .routing_key = Config::Routes::messageStatusSaved,
                           .message = nlohmann::json(saved_message_status).dump(),
                           .exchange_type = Config::Routes::exchangeType};

    mq_client_->publish(request);
}

