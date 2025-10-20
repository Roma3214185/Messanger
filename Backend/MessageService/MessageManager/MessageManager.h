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

    std::optional<MessageStatus> getMessageStatus(int message_id, int receiverId){
        qDebug() << "[getMessageStatus] messageId = " << message_id << " and receiverId = " << receiverId;

        auto q = msgRepo.query<MessageStatus>()
            .filter("id", message_id)
            .filter("receiver_id", receiverId)
            .limit(1);

        auto res = q.execute();
        LOG_INFO("Finded for message_id = '{}' and receiverId = '{}' status messages size = '{}'", message_id, receiverId, res.size());
        if(res.empty()) return std::nullopt;
        return res.front();
    }

    std::optional<int> getChatId(int messageId){
        auto message = getMessage(messageId);
        if(!message) return std::nullopt;
        return message->chat_id;
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
        // for(auto msg: messages){
        //     qDebug() << "Msg id = " << msg.id;
        //     qDebug() << "chat_id = " << msg.chat_id;
        //     qDebug() << "sender_id = " << msg.sender_id;
        //     qDebug() << "text = " << msg.text;
        // }


        //std::reverse(messages.begin(), messages.end());
        return messages;
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
