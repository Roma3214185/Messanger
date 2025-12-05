#ifndef IRABITMQCLIENT_H
#define IRABITMQCLIENT_H

#include <string>
#include <functional>

struct PublishRequest{
  std::string exchange;
  std::string routingKey;
  std::string message;
  std::string exchangeType = "direct";
};

struct SubscribeRequest{
    std::string queue;
    std::string exchange;
    std::string routingKey;
    std::string exchangeType = "direct";
};

class IRabitMQClient {
  public:
    using EventCallback = std::function<void(const std::string& event, const std::string& payload)>;
    virtual ~IRabitMQClient() = default;
    virtual void publish(const PublishRequest&) = 0;
    virtual void subscribe(const SubscribeRequest&, const EventCallback&) = 0;
    virtual void stop() = 0;
};


#endif // IRABITMQCLIENT_H
