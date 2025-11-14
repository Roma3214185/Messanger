#ifndef MOCKRABITMQCLIENT_H
#define MOCKRABITMQCLIENT_H

#include "interfaces/IRabitMQClient.h"

class MockRabitMQClient : public IRabitMQClient {
  public:
    void publish(const std::string& exchange,
                         const std::string& routingKey,
                         const std::string& message,
                         const std::string& exchangeType = "direct") override {

    }

    void subscribe(
        const std::string&                                                               queue,
        const std::string&                                                               exchange,
        const std::string&                                                               routingKey,
        const std::function<void(const std::string& event, const std::string& payload)>& callback,
        const std::string& exchangeType = "direct") override {

    }

    void stop() override {

    }

};

#endif // MOCKRABITMQCLIENT_H
