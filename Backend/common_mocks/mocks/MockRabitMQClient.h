#ifndef MOCKRABITMQCLIENT_H
#define MOCKRABITMQCLIENT_H

#include "interfaces/IRabitMQClient.h"

class MockRabitMQClient : public IRabitMQClient {
  public:
    virtual void publish(const PublishRequest& request) override {
      last_publish_request = request;
      publish_mp[request.routingKey]++;
      ++publish_cnt;
    }

    virtual void subscribe(const SubscribeRequest& request, const EventCallback& cb) override {
      last_callback = std::move(cb);
      last_subscribe_request = request;
      ++subscribe_cnt;
    }

    void stop() override {

    }

    void callLastCallback(const std::string& payload) {
      last_callback(last_subscribe_request.routingKey, payload);
    }

    int getPublishCnt(const std::string& routingKey) {
      return publish_mp[routingKey];
    }

    int publish_cnt = 0;
    int subscribe_cnt = 0;
    std::unordered_map<std::string, int> publish_mp;
    PublishRequest last_publish_request;
    SubscribeRequest last_subscribe_request;
    EventCallback last_callback;

};

#endif // MOCKRABITMQCLIENT_H
