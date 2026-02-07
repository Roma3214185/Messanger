#ifndef IPUBLISHER_H
#define IPUBLISHER_H

class MessageStatus;
class Message;
class Reaction;

class IPublisher {
 public:
  virtual ~IPublisher() = default;
  virtual void saveMessageStatus(MessageStatus &status) = 0;
  virtual void saveReaction(const Reaction &reaction) = 0;
  virtual void deleteReaction(const Reaction &reaction) = 0;
  virtual void saveMessage(const Message &message) = 0;
  virtual void deleteMessageStatus(const MessageStatus &message) = 0;
};

class IEventPublisher;

class RabbitNotificationPublisher : public IPublisher {
 public:
  RabbitNotificationPublisher(IEventPublisher *mq_client);

  void saveMessageStatus(MessageStatus &status) override;
  void saveReaction(const Reaction &reaction) override;
  void deleteReaction(const Reaction &reaction) override;
  void saveMessage(const Message &message) override;
  void deleteMessageStatus(const MessageStatus &message) override;

  private:
    IEventPublisher *mq_client_;
};

#endif  // IPUBLISHER_H
