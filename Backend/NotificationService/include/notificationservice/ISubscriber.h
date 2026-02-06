#ifndef ISUBSCRIBER_H
#define ISUBSCRIBER_H

class IEventSubscriber;
class NotificationOrchestrator;

class ISubscriber {
 public:
  virtual ~ISubscriber() = default;
  virtual void subscribeAll() = 0;
};

class RabbitNotificationSubscriber : public ISubscriber {
  NotificationOrchestrator* notification_orchestrator_;
  IEventSubscriber* mq_client_;

 public:
  RabbitNotificationSubscriber(IEventSubscriber* mq_client, NotificationOrchestrator* notification_orchestrator);
  void subscribeAll() override;

 protected:
  void subscribeMessageDeleted();
  void subscribeMessageReactionDeleted();
  void subscribeMessageReactionSaved();
  void subscribeMessageSaved();
  void subscribeMessageStatusSaved();
};

#endif  // ISUBSCRIBER_H
