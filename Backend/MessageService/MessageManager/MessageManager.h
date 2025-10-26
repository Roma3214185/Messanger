#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "GenericReposiroty.h"
#include "Message.h"
#include "GenericReposiroty.h"

class MessageManager
{
    GenericRepository& msgRepo;
    SaverBatcher<Message> saverBatcherMessages;
    SaverBatcher<MessageStatus> saverBatcherMessagesStatus;
    DeleterBatcher<Message> deleterBatcherMessages;
    DeleterBatcher<MessageStatus> deleterBatcherMessagesStatus;

    std::unique_ptr<Batcher<Message>> messagesBatcher;
    std::unique_ptr<Batcher<MessageStatus>> messagesStatusBatcher;
public:
    MessageManager(GenericRepository& rep)
        : msgRepo(rep)
        , saverBatcherMessages(rep)
        , saverBatcherMessagesStatus(rep)
        , deleterBatcherMessages(rep)
        , deleterBatcherMessagesStatus(rep)
    {
        messagesBatcher = std::make_unique<Batcher<Message>>(saverBatcherMessages, deleterBatcherMessages);
        messagesStatusBatcher = std::make_unique<Batcher<MessageStatus>>(saverBatcherMessagesStatus, deleterBatcherMessagesStatus);
    }

    void saveMessage(Message& msg) {
        PROFILE_SCOPE("MessageManager::saveMessage");
        messagesBatcher->save(msg);
        //msgRepo.save(msg);
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
        //msgRepo.save(status);
        messagesStatusBatcher->save(status);
    }

    std::vector<MessageStatus> getUndeliveredMessages(int userId){
        auto res = msgRepo.query<MessageStatus>()
                .filter("receiver_id", userId)
                     .filter("is_read", "0")
                     .execute();

        LOG_INFO("getUndeliveredMessages return '{}' messages for userId '{}'", res.size(), userId);
        return res;
    }
};

#endif // MESSAGEMANAGER_H
