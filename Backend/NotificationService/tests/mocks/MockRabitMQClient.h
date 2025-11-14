#ifndef MOCKRABITMQCLIENT_H
#define MOCKRABITMQCLIENT_H

#include "interfaces/IRabitMQClient.h"

class MockRabitMQClient : public IRabitMQClient {
  public:
    virtual void publish(const PublishRequest&) override {

    }

    virtual void subscribe(const SubscribeRequest&, const EventCallback&) override {

    }

    void stop() override {

    }

};

#endif // MOCKRABITMQCLIENT_H
