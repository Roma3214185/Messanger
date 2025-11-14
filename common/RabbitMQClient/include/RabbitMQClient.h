#pragma once

#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "Debug_profiling.h"
#include "ThreadPool.h"
#include "interfaces/IRabitMQClient.h"

struct RabbitMQConfig {
    std::string host;
    int         port;
    std::string user;
    std::string password;
};

class RabbitMQClient : public IRabitMQClient {
 public:
  RabbitMQClient(const RabbitMQConfig&, size_t thread_pool_size = 4);
  ~RabbitMQClient();

  void publish(const PublishRequest&) override;
  void subscribe(const SubscribeRequest&, const EventCallback&) override;
  void stop();

 private:
  void declareExchange(const std::string& exchange, const std::string& type, bool durable);

 private:
  std::atomic<bool>               running_{false};
  ThreadPool                      pool_;
  std::vector<std::thread>        consumer_threads_;
  std::mutex                      consumer_threads_mutex_;
  std::unordered_set<std::string> declared_exchanges_;
  const RabbitMQConfig&           rabit_mq_config_;
};
