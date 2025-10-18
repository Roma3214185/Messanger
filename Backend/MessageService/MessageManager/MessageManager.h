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

    std::vector<Message> getChatMessages(int chatId, int limit, int beforeId) const{
        auto q = msgRepo.query<Message>()
                    .filter("chat_id", chatId)
                    .limit(limit)
                    .orderBy("timestamp", "DESC");


        if (beforeId > 0) {
            q.filter("id", "<", beforeId); // add older messages filter
        }

        auto messages = q.execute();
        //std::reverse(messages.begin(), messages.end());
        return messages;
    }

    void saveMessageStatus(MessageStatus& status){
        msgRepo.save(status);
    }

    std::vector<MessageStatus> getUndeliveredMessages(int userId){
        auto res = msgRepo.query<MessageStatus>()
                .filter("receiver_id", userId)
                     .filter("is_read", "0")
                     .execute();

        LOG_INFO("getUndeliveredMessages return '{}' messages for userId '{}'", res.size(), userId);
        return res;
    }

private:
    GenericRepository& msgRepo;
};

#endif // MESSAGEMANAGER_H
