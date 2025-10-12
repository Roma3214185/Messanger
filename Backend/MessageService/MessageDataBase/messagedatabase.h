#ifndef DATABASE_H
#define DATABASE_H

#include <QList>
#include <optional>
#include <QSqlQuery>

#include "Message.h"

using MessageId = int;
using OptinonalMessage = std::optional<Message>;
using OptionalMessageId = std::optional<MessageId>;

class QSqlDatabase;

class DataBase{

public:

    void clearDataBase();
    OptionalMessageId addMsgToDatabase(const std::string& messageText, int fromUserId, int chatId);
    bool initialDb();
    QList<Message> getUndeliveredMessages(int userId);
    void markDelivered(int msgId);
    void saveMessage(int msgId, int senderid, int receiverId, const std::string& text, bool delivered);
    QList<Message> getChatMessages(int chatId);
    OptinonalMessage saveSendedMessage(int chatId, int senderId, std::string text);

private:
    QSqlDatabase getThreadDatabase();
    Message getMessageFromQuery(const QSqlQuery& query);
    template<typename... Args>
    bool executeQuery(QSqlQuery& query, Args&&... args);
};

#endif // DATABASE_H
