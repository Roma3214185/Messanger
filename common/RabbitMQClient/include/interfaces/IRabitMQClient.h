#ifndef IRABITMQCLIENT_H
#define IRABITMQCLIENT_H

#include <functional>
#include <string>

struct PublishRequest {
  std::string exchange;
  std::string routing_key;
  std::string message;
  std::string exchange_type = "direct";
};

struct SubscribeRequest {
  std::string queue;
  std::string exchange;
  std::string routing_key;
  std::string exchange_type = "direct";
};

class IEventBusLifecycle {
 public:
  virtual ~IEventBusLifecycle() = default;
  virtual void stop() = 0;
};

class IEventSubscriber {
 public:
  using EventCallback = std::function<void(const std::string &, const std::string &)>;
  virtual ~IEventSubscriber() = default;
  virtual void subscribe(const SubscribeRequest &, const EventCallback &) = 0;
};

class IEventPublisher {
 public:
  virtual ~IEventPublisher() = default;
  virtual void publish(const PublishRequest &) = 0;
};

class IEventBus : public IEventPublisher, public IEventSubscriber {
 public:
  virtual ~IEventBus() = default;
};

#endif  // IRABITMQCLIENT_H
