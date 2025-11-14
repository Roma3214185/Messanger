#ifndef IRABITMQCLIENT_H
#define IRABITMQCLIENT_H

#include <string>
#include <functional>

class IRabitMQClient {
  public:
    virtual ~IRabitMQClient() = default;
    virtual void publish(const std::string& exchange,
                 const std::string& routingKey,
                 const std::string& message,
                 const std::string& exchangeType = "direct") = 0;

    virtual void subscribe(
        const std::string&                                                               queue,
        const std::string&                                                               exchange,
        const std::string&                                                               routingKey,
        const std::function<void(const std::string& event, const std::string& payload)>& callback,
        const std::string& exchangeType = "direct") = 0;

    virtual void stop() = 0;
};


#endif // IRABITMQCLIENT_H
