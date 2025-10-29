#ifndef BACKEND_RABBITMQCLIENT_RABBITMQCLIENT_H_
#define BACKEND_RABBITMQCLIENT_RABBITMQCLIENT_H_

#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <atomic>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_set>

#include "Debug_profiling.h"
#include "ThreadPool.h"
//#include "../Backend/GenericRepository/ThreadPool.h"

class RabbitMQClient {
 public:
  RabbitMQClient(const std::string& host, int port, const std::string& user,
                 const std::string& password, size_t thread_pool_size = 4);

  ~RabbitMQClient();

  void publish(const std::string& exchange, const std::string& routingKey,
               const std::string& message,
               const std::string& exchangeType = "direct");

  void subscribe(
      const std::string& queue, const std::string& exchange,
      const std::string& routingKey,
      const std::function<void(const std::string& event,
                               const std::string& payload)>& callback,
      const std::string& exchangeType = "direct");

  void stop();

 private:
  AmqpClient::Channel::ptr_t channel_;
  std::thread consumer_thread_;
  std::atomic<bool> running_{false};
  std::unordered_set<std::string> declared_exchanges_;
  ThreadPool pool_;

  void declareExchange(const std::string& exchange, const std::string& type,
                       bool durable = true);
};

#endif  // BACKEND_RABBITMQCLIENT_RABBITMQCLIENT_H_
