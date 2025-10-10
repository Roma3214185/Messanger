#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <QList>
#include <QDateTime>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

struct Chat{
    int id;
    bool isGroup;
    std::string name;
    std::string avatar;
    QDateTime createdAt;
};

// struct Message{
//     chatJson["last_message"] = lastMsg->text;
//     chatJson["timestamp"] = lastMsg->timestamp.toStdString();
// };

class DataBase
{
public:
    //DataBase();
    void clearDataBase();
    std::optional<int> createPrivateChat();
    bool addMembersToChat(int chatId, const std::vector<int>& membersId);
    bool initialDb();
    //bool connectDb();
    bool deleteChat(int charId);
    bool deleteMembersFromChat(int chatId, const std::vector<int>& membersId);
    std::optional<QList<int>> getMembersOfChat(int chatId);
    QList<Chat> getChatsOfUser(int id);
    int getMembersCount(int chat_id);
    //std::optional<Message> getLastMessage(int chat_id);
    std::optional<int> getOtherMemberId(int chat_id, int userId);
    std::optional<Chat> getChatById(int chatId);
};

#endif // DATABASE_H
