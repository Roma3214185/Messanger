#ifndef MOCKRABITMQCLIENT_H
#define MOCKRABITMQCLIENT_H

#include "interfaces/IRabitMQClient.h"
#include "Debug_profiling.h"

class MockRabitMQClient : public IRabitMQClient {
  public:
    virtual void publish(const PublishRequest& request) override {
      last_publish_request = request;
      publish_mp[request.routingKey]++;
      if(call_backs.count(request.routingKey)) call_backs[request.routingKey](request.routingKey, request.message);
      else LOG_INFO("Not found callback for {}", request.routingKey);
      ++publish_cnt;
    }

    virtual void subscribe(const SubscribeRequest& request, const EventCallback& cb) override {
      last_callback = cb;
      last_subscribe_request = request;
      call_backs[request.routingKey] = cb;
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
    std::unordered_map<std::string, EventCallback> call_backs;
    std::unordered_map<std::string, int> publish_mp;
    PublishRequest last_publish_request;
    SubscribeRequest last_subscribe_request;
    EventCallback last_callback;

};

#endif // MOCKRABITMQCLIENT_H
