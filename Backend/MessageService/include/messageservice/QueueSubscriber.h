#ifndef QUEUESUBSCRIBER_H
#define QUEUESUBSCRIBER_H

#include <string>
#include <functional>

class IEventSubscriber;

class QueueSubscriber {
 public:
    using Callback = const std::function<void(std::string)>&;
    QueueSubscriber(IEventSubscriber *mq_client);

    void subscribeToSaveMessage(Callback);
    void subscribeToSaveMessageStatus(Callback);
    void subscribeToSaveMessageReaction(Callback);
    void subscribeToDeleteMessageReaction(Callback);

 private:
    IEventSubscriber* mq_client_;
};


#endif // QUEUESUBSCRIBER_H
