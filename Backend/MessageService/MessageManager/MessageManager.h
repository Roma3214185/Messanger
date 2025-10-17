#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H
#include "../../GenericRepository/GenericReposiroty.h"
#include "../Headers/Message.h"
#include "../../GenericRepository/GenericReposiroty.h"

class MessageManager {
public:
    MessageManager(GenericRepository& msgRepo)
        : msgRepo(msgRepo) {}

    void saveMessage(Message& msg) {
        msgRepo.save(msg);
        //statusRepo.saveMessage(msg.id, msg.sender_id, msg.receiver_id, msg.text, false); // save as undelivered, id is updated???
    }

    std::optional<Message> getMessage(int id){
        return msgRepo.findOne<Message>(id);
    }

    // void markDelivered(const MessageStatus& msg) {
    //     msgRepo.save(msg);
    //     //statusRepo.markDelivered(msg.id, msg.receiver_id);
    // }

    std::vector<Message> getChatMessages(int chatId){
        return msgRepo.findBy<Message>("chat_id", chatId);
    }

    void saveMessageStatus(MessageStatus& status){
        msgRepo.save(status);
    }

    std::vector<MessageStatus> getUndeliveredMessages(int userId){
        return msgRepo.findBy<MessageStatus>("receiver_id", userId);
    }

private:
    GenericRepository& msgRepo;
};

#endif // MESSAGEMANAGER_H
