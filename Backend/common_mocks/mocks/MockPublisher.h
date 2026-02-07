#ifndef MOCKPUBLISHER_H
#define MOCKPUBLISHER_H

#include "notificationservice/IPublisher.h"
#include "entities/MessageStatus.h"
#include "entities/Reaction.h"
#include "entities/Message.h"

class MockPublisher : public IPublisher {
public:
    int calls_saveMessageStatus = 0;
    std::vector<MessageStatus> messages_status_to_save;
    void saveMessageStatus(MessageStatus &status) override {
        ++calls_saveMessageStatus;
        messages_status_to_save.push_back(status);
    }

    int calls_saveReaction = 0;
    std::vector<Reaction> reactions_to_save;
    void saveReaction(const Reaction &reaction) override {
        ++calls_saveMessage;
        reactions_to_save.push_back(reaction);
    }

    int calls_deleteReaction = 0;
    std::vector<Reaction> reactions_to_delete;
    void deleteReaction(const Reaction &reaction) override {
        ++calls_deleteReaction;
        reactions_to_delete.push_back(reaction);
    }

    int calls_saveMessage = 0;
    std::vector<Message> messages_to_save;
    void saveMessage(const Message &message) override {
        ++calls_saveMessage;
        messages_to_save.push_back(message);
    }

    int calls_deleteMessageStatus = 0;
    std::vector<MessageStatus> messages_status_to_delete;
    void deleteMessageStatus(const MessageStatus &message) override {
        ++calls_deleteMessageStatus;
        messages_status_to_delete.push_back(message);
    }
};

#endif // MOCKPUBLISHER_H
