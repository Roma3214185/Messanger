#ifndef QUEUEPUBLISHER_H
#define QUEUEPUBLISHER_H

class IEventPublisher;
struct Message;
struct Reaction;
struct MessageStatus;

class QueuePublisher {
 public:
   QueuePublisher(IEventPublisher* mq_client);
   void messageSaved(const Message& saved_message);
   void messageDeleted(const Message& deleted_message);
   void reactionDeleted(const Reaction& deleted_reaction);
   void reactionSaved(const Reaction& saved_reaction);
   void messageStatusSaved(const MessageStatus& saved_message_status);

 private:
  IEventPublisher* mq_client_;
};

#endif // QUEUEPUBLISHER_H
