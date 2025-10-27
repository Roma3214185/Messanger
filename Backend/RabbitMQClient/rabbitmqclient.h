#ifndef BACKEND_RABBITMQCLIENT_RABBITMQCLIENT_H_
#define BACKEND_RABBITMQCLIENT_RABBITMQCLIENT_H_

#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <atomic>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

class RabbitMQClient {
 public:
  RabbitMQClient(const std::string& host, const std::string& user,
                 const std::string& password, const std::string& vhost = "/",
                 int port = 5672);

  ~RabbitMQClient();

  void publish(const std::string& exchange, const std::string& routing_key,
               const std::string& message);

  void subscribe(const std::string& queue,
                 const std::function<void(const std::string&)>& callback);

 private:
  void consumerLoop(const std::string& queue,
                    std::function<void(const std::string&)> callback);

  AmqpClient::Channel::ptr_t channel_;
  std::thread consumer_thread_;
  std::atomic<bool> running_{false};
};

#endif  // BACKEND_RABBITMQCLIENT_RABBITMQCLIENT_H_
