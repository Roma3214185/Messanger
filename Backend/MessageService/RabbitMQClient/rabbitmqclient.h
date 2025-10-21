#ifndef RABBITMQCLIENT_H
#define RABBITMQCLIENT_H

#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <nlohmann/json.hpp>
#include <functional>
#include <string>

class RabbitMQClient
{
public:
    RabbitMQClient(const std::string& host);

    void publish(const std::string& exchange, const std::string& message);
    void subscribe(const std::string& queue, std::function<void(const std::string&)> callback);

private:
    AmqpClient::Channel::ptr_t channel_;
};


#endif // RABBITMQCLIENT_H
