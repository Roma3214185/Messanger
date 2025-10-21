#include "rabbitmqclient.h"

RabbitMQClient::RabbitMQClient(const std::string& host) {
    channel_ = AmqpClient::Channel::Create(host);
}

void RabbitMQClient::subscribe(const std::string& queue, std::function<void(const std::string&)> callback) {
    channel_->DeclareQueue(queue);
    channel_->BasicConsume(queue);

    while (true) {
        AmqpClient::Envelope::ptr_t envelope;
        if (channel_->BasicConsumeMessage(envelope, 1000)) {
            callback(envelope->Message()->Body());
        }
    }
}

void RabbitMQClient::publish(const std::string& exchange, const std::string& message) {
    channel_->BasicPublish(exchange, "", AmqpClient::BasicMessage::Create(message));
}
