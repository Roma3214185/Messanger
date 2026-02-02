#ifndef IPUBLISHER_H
#define IPUBLISHER_H

class MessageStatus;
class Message;
class Reaction;

class IPublisher {
public:
    virtual ~IPublisher() = default;
    virtual void saveMessageStatus(MessageStatus &status) = 0;
    virtual void saveDeliveryStatus(const Message &msg, long long receiver_id) = 0;
    virtual void saveReaction(const Reaction &reaction) = 0;
    virtual void deleteReaction(const Reaction &reaction) = 0;
    virtual void saveMessage(const Message &message) = 0;
};

class IEventPublisher;

class RabbitNotificationPublisher : public IPublisher {
    IEventPublisher* mq_client_;
public:
    RabbitNotificationPublisher(IEventPublisher* mq_client);
private:
    void saveMessageStatus(MessageStatus &status) override;
    void saveDeliveryStatus(const Message &msg, long long receiver_id) override;
    void saveReaction(const Reaction &reaction) override;
    void deleteReaction(const Reaction &reaction) override;
    void saveMessage(const Message &message) override;
};

#endif // IPUBLISHER_H
