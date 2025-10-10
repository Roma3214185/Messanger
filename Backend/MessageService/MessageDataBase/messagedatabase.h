#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <optional>
#include <QList>
#include <string>
#include <iostream>
#include <QDateTime>

struct Message{
    int id;
    int chatId;
    int senderId;
    std::string text;
    int receiverId;
    QString timestamp;
};

class DataBase
{
public:
    void clearDataBase();
    std::optional<int> addMsgToDatabase(const std::string& messageText, int fromUserId, int chatId);
    bool initialDb();
    QList<Message> getUndeliveredMessages(int userId);
    void markDelivered(int msgId);
    void saveMessage(int msgId, int fromUser, int toUser, const std::string& text, bool delivered);
    QList<Message> getChatMessages(int chatId);
    std::optional<Message> saveSendedMessage(int chatId, int sender_id, std::string text);
};

#endif // DATABASE_H
