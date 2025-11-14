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

class RabbitMQClient : public IRabitMQClient {
 public:
  RabbitMQClient(const std::string& host,
                 int                port,
                 const std::string& user,
                 const std::string& password,
                 size_t             thread_pool_size = 4);
  ~RabbitMQClient();

  void publish(const std::string& exchange,
               const std::string& routingKey,
               const std::string& message,
               const std::string& exchangeType = "direct") override;

  void subscribe(
      const std::string&                                                               queue,
      const std::string&                                                               exchange,
      const std::string&                                                               routingKey,
      const std::function<void(const std::string& event, const std::string& payload)>& callback,
      const std::string& exchangeType = "direct") override;

  void stop();

 private:
  void declareExchange(const std::string& exchange, const std::string& type, bool durable);

 private:
  std::atomic<bool>               running_{false};
  ThreadPool                      pool_;
  std::vector<std::thread>        consumer_threads_;
  std::mutex                      consumer_threads_mutex_;
  std::unordered_set<std::string> declared_exchanges_;
  std::string                     host_, user_, password_;
  int                             port_;
};
