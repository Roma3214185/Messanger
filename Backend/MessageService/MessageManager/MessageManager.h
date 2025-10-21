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
    }

    std::vector<MessageStatus> getMessageStatusDebug(){
        auto res =  msgRepo.query<MessageStatus>()
                       .filter("receiver_id", 3)
                        .execute();
        qDebug() << "[getMessageStatusDebug] return size = " << res.size();
        return res;
    }

    std::optional<Message> getMessage(int id){
        return msgRepo.findOne<Message>(id);
    }

    std::optional<MessageStatus> getMessageStatus(int message_id, int receiverId) {
        auto q = msgRepo.query<MessageStatus>()
            .filter("id", message_id)
            .filter("receiver_id", receiverId)
            .limit(1);

        auto res = q.execute();
        LOG_INFO("Found for message_id = '{}' and receiverId = '{}' status messages size = '{}'",
                 message_id, receiverId, res.size());

        if(res.empty()) return std::nullopt;
        return res.front();
    }

    std::optional<int> getChatId(int messageId){
        auto message = getMessage(messageId);
        if(!message) return std::nullopt;
        return message->chat_id;
    }

    std::vector<Message> getChatMessages(int chatId, int limit, int beforeId) {
        auto q = msgRepo.query<Message>()
            .filter("chat_id", chatId)
            .orderBy("timestamp", "DESC")
            .limit(limit);

        if (beforeId > 0) {
            q.filter("id", "<", beforeId);
        }

        return q.execute();
    }

    void saveMessageStatus(MessageStatus& status){
        qDebug() << "[saveMessageStatus] messageId = " << status.id << " and receiverId = " << status.receiver_id;
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
