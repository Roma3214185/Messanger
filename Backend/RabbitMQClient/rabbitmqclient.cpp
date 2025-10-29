#include "RabbitMQClient.h"

#include <string>

#include "Debug_profiling.h"

RabbitMQClient::RabbitMQClient(const std::string& host, int port,
                               const std::string& user,
                               const std::string& password,
                               size_t thread_pool_size)
    : pool_(thread_pool_size) {
  channel_ = AmqpClient::Channel::Create(host, port, user, password);
}

RabbitMQClient::~RabbitMQClient() { stop(); }

void RabbitMQClient::publish(const std::string& exchange,
                             const std::string& routingKey,
                             const std::string& message,
                             const std::string& exchangeType) {
  declareExchange(exchange, exchangeType, true);
  LOG_INFO("[rabbit] Publishing to exchange '{}' with key '{}': {}", exchange,
           routingKey, message);

  auto msg = AmqpClient::BasicMessage::Create(message);
  msg->DeliveryMode(AmqpClient::BasicMessage::dm_persistent);
  channel_->BasicPublish(exchange, routingKey, msg);
}

void RabbitMQClient::subscribe(
    const std::string& queue, const std::string& exchange,
    const std::string& routingKey,
    const std::function<void(const std::string& event,
                             const std::string& payload)>& callback,
    const std::string& exchangeType) {
  declareExchange(exchange, exchangeType, true);
  LOG_INFO("[rabbit] Subscribed to exchange '{}' with key '{}'", exchange,
           routingKey);

  channel_->DeclareQueue(queue, true, false, false, false);
  LOG_INFO("Bind queue: {} exchange {} key {}", queue, exchange, routingKey);
  channel_->BindQueue(queue, exchange, routingKey);

  std::string consumerTag =
      channel_->BasicConsume(queue, "", false, false, false);
  running_ = true;

  consumer_thread_ = std::thread([this, callback, consumerTag]() {
    while (running_) {
      try {
        AmqpClient::Envelope::ptr_t envelope;
        if (channel_->BasicConsumeMessage(consumerTag, envelope, 100)) {
          std::string event = envelope->RoutingKey();
          std::string payload = envelope->Message()->Body();

          pool_.enqueue([this, callback, envelope, event, payload]() {
            callback(event, payload);
            channel_->BasicAck(envelope);
          });
        }
      } catch (const AmqpClient::AmqpException& e) {
        LOG_ERROR("Consumer error: {}", e.what());
      }
    }
  });
}

void RabbitMQClient::stop() {
  running_ = false;
  if (consumer_thread_.joinable()) {
    consumer_thread_.join();
  }
}

void RabbitMQClient::declareExchange(const std::string& exchange,
                                     const std::string& type,
                                     bool durable) {
  if (declared_exchanges_.count(exchange)) return;
  try {
    channel_->DeclareExchange(exchange, type, durable, true, false);
    declared_exchanges_.insert(exchange);
  } catch (const AmqpClient::AmqpException& e) {
    LOG_WARN("Exchange '{}' might already exist: {}", exchange, e.what());
  }
}
