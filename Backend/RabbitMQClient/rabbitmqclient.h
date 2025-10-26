#ifndef RABBITMQCLIENT_H
#define RABBITMQCLIENT_H

#include <atomic>
#include <string>
#include <functional>
#include <thread>

#include <nlohmann/json.hpp>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

class RabbitMQClient {
public:
    RabbitMQClient(const std::string& host,
                   const std::string& user,
                   const std::string& password,
                   const std::string& vhost = "/",
                   int port = 5672);

    ~RabbitMQClient();

    void publish(const std::string& exchange,
                 const std::string& routingKey,
                 const std::string& message);

    void subscribe(const std::string& queue,
                   const std::function<void(const std::string&)>& callback);

private:
    void consumerLoop(const std::string& queue,
                      std::function<void(const std::string&)> callback);

    AmqpClient::Channel::ptr_t channel_;
    std::thread consumerThread_;
    std::atomic<bool> running_{false};
};

#endif // RABBITMQCLIENT_H
