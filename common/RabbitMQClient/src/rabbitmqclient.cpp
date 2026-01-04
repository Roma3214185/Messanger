#include "RabbitMQClient.h"

#include "Debug_profiling.h"

RabbitMQClient::RabbitMQClient(const RabbitMQConfig& rabit_mq_config, IThreadPool* pool)
    : pool_(pool), rabit_mq_config_(rabit_mq_config) {}

RabbitMQClient::~RabbitMQClient() { stop(); }

void RabbitMQClient::declareExchange(const std::string& exchange,
                                     const std::string& type,
                                     bool               durable) {
  if (declared_exchanges_.contains(exchange)) {
    LOG_INFO("{} already exist", exchange);
    return;
  }

  try {
    auto channel = AmqpClient::Channel::Create(rabit_mq_config_.host,
                                               rabit_mq_config_.port,
                                               rabit_mq_config_.user,
                                               rabit_mq_config_.password);
    channel->DeclareExchange(exchange, type, durable, false, false);
    declared_exchanges_.insert(exchange);
    LOG_INFO("[rabbit] Declared exchange '{}'", exchange);
  } catch (const AmqpClient::AmqpException& e) {
    LOG_WARN("[rabbit] Error while declaring '{}': {}", exchange, e.what());
  }
}

void RabbitMQClient::publish(const PublishRequest& publish_request) {
  LOG_INFO("Publish: {} | {}", publish_request.exchange, publish_request.routing_key);
  declareExchange(publish_request.exchange, publish_request.exchange_type, false);
  try {
    auto channel = AmqpClient::Channel::Create(rabit_mq_config_.host,
                                               rabit_mq_config_.port,
                                               rabit_mq_config_.user,
                                               rabit_mq_config_.password);
    auto msg     = AmqpClient::BasicMessage::Create(publish_request.message);
    msg->DeliveryMode(AmqpClient::BasicMessage::dm_persistent);
    LOG_INFO("[rabbit] Try to publish message '{}' to exchange '{}' with key '{}'",
             publish_request.message,
             publish_request.exchange,
             publish_request.exchange_type);
    channel->BasicPublish(publish_request.exchange, publish_request.routing_key, msg);
    LOG_INFO("[rabbit] Published message '{}' to exchange '{}' with key '{}'",
             publish_request.message,
             publish_request.exchange,
             publish_request.exchange_type);
  } catch (const std::exception& e) {
    LOG_ERROR("[rabbit] Publish failed: {}", e.what());
  }
}

void RabbitMQClient::subscribe(const SubscribeRequest& subscribe_request,
                               const EventCallback&    callback) {
  LOG_INFO("Subscrive: {} | {}", subscribe_request.exchange, subscribe_request.routing_key);
  running_ = true;

  auto consumer_thread = std::thread([=, this]() {
    try {
      declareExchange(subscribe_request.exchange, subscribe_request.exchange_type, false);
      auto channel = AmqpClient::Channel::Create(rabit_mq_config_.host,
                                                 rabit_mq_config_.port,
                                                 rabit_mq_config_.user,
                                                 rabit_mq_config_.password);
      // channel->DeclareExchange(subscribe_request.exchange, subscribe_request.exchangeType, true,
      // false, false);
      channel->DeclareQueue(subscribe_request.queue, false, false, false, false);
      channel->BindQueue(
          subscribe_request.queue, subscribe_request.exchange, subscribe_request.routing_key);

      LOG_INFO("[rabbit] Queue '{}' bound to exchange '{}' with key '{}'",
               subscribe_request.queue,
               subscribe_request.exchange,
               subscribe_request.routing_key);

      const std::string consumer_tag =
          channel->BasicConsume(subscribe_request.queue, "", false, false, false);
      LOG_INFO("[rabbit] Subscribed to '{}':'{}' in queue '{}'",
               subscribe_request.exchange,
               subscribe_request.routing_key,
               subscribe_request.queue);

      while (running_) {
        AmqpClient::Envelope::ptr_t envelope;
        if (!channel->BasicConsumeMessage(consumer_tag, envelope, 200)) continue;

        std::string event   = envelope->RoutingKey();
        std::string payload = envelope->Message()->Body();
        LOG_INFO("[rabbit] Received payload: {}", payload);
        LOG_INFO("[rabbit] Received event: {}", event);

        pool_->enqueue([callback, event, payload]() {
          try {
            callback(event, payload);
          } catch (const std::exception& e) {
            LOG_ERROR("[rabbit] Callback error: {}", e.what());
          }
        });

        channel->BasicAck(envelope);
      }
    } catch (const AmqpClient::AmqpException& e) {
      LOG_ERROR("[rabbit] Consumer error: {}", e.what());
    }
  });

  std::lock_guard<std::mutex> lock(consumer_threads_mutex_);
  consumer_threads_.emplace_back(std::move(consumer_thread));
}

void RabbitMQClient::stop() {
  running_ = false;
  std::lock_guard<std::mutex> lock(consumer_threads_mutex_);
  for (auto& thread : consumer_threads_) {
    if (thread.joinable()) thread.join();
  }
  consumer_threads_.clear();
}
