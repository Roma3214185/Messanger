#ifndef RABBITMQCLIENT
#define RABBITMQCLIENT

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
#include "interfaces/IRabitMQClient.h"
#include "threadpool.h"

struct RabbitMQConfig {
  std::string host;
  int port;
  std::string user;
  std::string password;
};

struct IThreadPool;

class RabbitMQClient : public IRabitMQClient {
 public:
  RabbitMQClient(const RabbitMQConfig &rabbitmq_config, IThreadPool *thread_pool);
  ~RabbitMQClient();

  RabbitMQClient(const RabbitMQClient &) = delete;
  RabbitMQClient &operator=(const RabbitMQClient &) = delete;

  RabbitMQClient(RabbitMQClient &&) = delete;
  RabbitMQClient &operator=(RabbitMQClient &&) = delete;

  void publish(const PublishRequest &publish_request) override;
  void subscribe(const SubscribeRequest &subscribe_request, const EventCallback &callback) override;
  void stop() override;

 private:
  void declareExchange(const std::string &exchange, const std::string &type, bool durable);

  std::atomic<bool> running_{false};
  IThreadPool *pool_;
  std::vector<std::thread> consumer_threads_;
  std::mutex consumer_threads_mutex_;
  std::unordered_set<std::string> declared_exchanges_;
  const RabbitMQConfig &rabit_mq_config_;
};

#endif  // RABBITMQCLIENT
