#include "RabbitMQClient.h"

RabbitMQClient::RabbitMQClient(const std::string& host, int port,
                               const std::string& user,
                               const std::string& password,
                               size_t thread_pool_size)
    : pool_(thread_pool_size), host_(host), port_(port), user_(user), password_(password) {}

RabbitMQClient::~RabbitMQClient() { stop(); }

void RabbitMQClient::declareExchange(const std::string& exchange,
                                     const std::string& type,
                                     bool durable) {
  if (declared_exchanges_.count(exchange)) return;
  try {
    auto channel = AmqpClient::Channel::Create(host_, port_, user_, password_);
    channel->DeclareExchange(exchange, type, durable, false, false);
    declared_exchanges_.insert(exchange);
    LOG_INFO("[rabbit] Declared exchange '{}'", exchange);
  } catch (const AmqpClient::AmqpException& e) {
    LOG_WARN("[rabbit] Exchange '{}' might already exist: {}", exchange, e.what());
  }
}

void RabbitMQClient::publish(const std::string& exchange,
                             const std::string& routingKey,
                             const std::string& message,
                             const std::string& exchangeType) {
  declareExchange(exchange, exchangeType, true);
  try {
    auto channel = AmqpClient::Channel::Create(host_, port_, user_, password_);
    auto msg = AmqpClient::BasicMessage::Create(message);
    msg->DeliveryMode(AmqpClient::BasicMessage::dm_persistent);
    channel->BasicPublish(exchange, routingKey, msg);
    LOG_INFO("[rabbit] Published message '{}' to exchange '{}' with key '{}'",
             message, exchange, routingKey);
  } catch (const std::exception& e) {
    LOG_ERROR("[rabbit] Publish failed: {}", e.what());
  }
}

void RabbitMQClient::subscribe(
    const std::string& queue,
    const std::string& exchange,
    const std::string& routingKey,
    const std::function<void(const std::string& event, const std::string& payload)>& callback,
    const std::string& exchangeType)
{
  running_ = true;

  auto consumer_thread = std::thread([=]() {
    try {
      auto channel = AmqpClient::Channel::Create(host_, port_, user_, password_);

      // Declare exchange (safe even if it already exists)
      channel->DeclareExchange(exchange, exchangeType, true, false, false);

      // Declare queue (durable)
      channel->DeclareQueue(queue, false, false, false, false);

      // Bind queue to exchange with routing key
      channel->BindQueue(queue, exchange, routingKey);

      LOG_INFO("[rabbit] Queue '{}' bound to exchange '{}' with key '{}'",
               queue, exchange, routingKey);

      // Start consuming
      std::string consumerTag = channel->BasicConsume(queue, "", false, false, false);
      LOG_INFO("[rabbit] Subscribed to '{}':'{}' in queue '{}'", exchange, routingKey, queue);

      while (running_) {
        AmqpClient::Envelope::ptr_t envelope;
        if (!channel->BasicConsumeMessage(consumerTag, envelope, 200)) continue;

        std::string event = envelope->RoutingKey();
        std::string payload = envelope->Message()->Body();
        LOG_INFO("[rabbit] Received payload: {}", payload);
        LOG_INFO("[rabbit] Received event: {}", event);
        pool_.enqueue([callback, event, payload]() {
          try { callback(event, payload); }
          catch (const std::exception& e) {
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
  for (auto& t : consumer_threads_) if (t.joinable()) t.join();
  consumer_threads_.clear();
}
