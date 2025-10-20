#ifndef RABBITMQCLIENT_H
#define RABBITMQCLIENT_H

#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <nlohmann/json.hpp>
#include <functional>
#include <string>

class RabbitMQClient {
public:
    RabbitMQClient(const std::string& host) {
        channel_ = AmqpClient::Channel::Create(host);
    }

    void publish(const std::string& exchange, const std::string& message) {
        // Публікація повідомлення в обмін
        channel_->BasicPublish(exchange, "", AmqpClient::BasicMessage::Create(message));
    }

    void subscribe(const std::string& queue, std::function<void(const std::string&)> callback) {
        // Створення черги
        channel_->DeclareQueue(queue);
        // Підписка на чергу
        channel_->BasicConsume(queue);
        // Обробка повідомлень
        while (true) {
            AmqpClient::Envelope::ptr_t envelope;
            if (channel_->BasicConsumeMessage(envelope, 1000)) {
                callback(envelope->Message()->Body());
            }
        }
    }

private:
    AmqpClient::Channel::ptr_t channel_;
};


#endif // RABBITMQCLIENT_H
