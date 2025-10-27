#include "RabbitMQClient.h"

#include <string>

#include "Debug_profiling.h"

RabbitMQClient::RabbitMQClient(const std::string& host, const std::string& user,
                               const std::string& password,
                               const std::string& vhost, int port)
    : channel_(AmqpClient::Channel::Create(host, port, user, password, vhost)) {
  LOG_INFO("[RabbitMQClient] Connected to '{}': '{}'", host, port);
}

RabbitMQClient::~RabbitMQClient() {
  running_ = false;
  if (consumer_thread_.joinable()) {
    consumer_thread_.join();
  }
}

void RabbitMQClient::publish(const std::string& exchange,
                             const std::string& routingKey,
                             const std::string& message) {
  AmqpClient::BasicMessage::ptr_t msg =
      AmqpClient::BasicMessage::Create(message);
  channel_->BasicPublish(exchange, routingKey, msg);
}

void RabbitMQClient::subscribe(
    const std::string& queue,
    const std::function<void(const std::string&)>& callback) {
  channel_->DeclareQueue(queue, false, true, false, false);
  std::string consumerTag =
      channel_->BasicConsume(queue, "", true, false, false);
  running_ = true;

  consumer_thread_ =
      std::thread(&RabbitMQClient::consumerLoop, this, consumerTag, callback);
}

void RabbitMQClient::consumerLoop(
    const std::string& consumerTag,
    std::function<void(const std::string&)> callback) {
  while (running_) {
    AmqpClient::Envelope::ptr_t envelope =
        channel_->BasicConsumeMessage(consumerTag);
    if (envelope) {
      std::string body = envelope->Message()->Body();
      callback(body);
    }
  }
}
