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

class IThreadPool;

class RabbitMQClient : public IRabitMQClient {
 public:
  RabbitMQClient(const RabbitMQConfig&, IThreadPool* pool);
  ~RabbitMQClient();

  void publish(const PublishRequest&) override;
  void subscribe(const SubscribeRequest&, const EventCallback&) override;
  void stop() override;

 private:
  void declareExchange(const std::string& exchange, const std::string& type, bool durable);

 private:
  std::atomic<bool>               running_{ false };
   IThreadPool*                   pool_;
  std::vector<std::thread>        consumer_threads_;
  std::mutex                      consumer_threads_mutex_;
  std::unordered_set<std::string> declared_exchanges_;
  const RabbitMQConfig&           rabit_mq_config_;
};
