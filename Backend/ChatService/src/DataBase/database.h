#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <QList>

#include "headers/Chat.h"
#include "GenericRepository.h"

using ChatId = int;
using UserId = int;
using OptionalChatId = std::optional<int>;
using OptionalChat = std::optional<Chat>;
using OptionalUserId = std::optional<UserId>;

class DataBase {
public:
    void clearDataBase();
    OptionalChatId createPrivateChat();
    bool addMembersToChat(int chatId, const std::vector<int>& membersId);
    bool initialDb();
    bool deleteChat(int charId);
    bool deleteMembersFromChat(int chatId, const std::vector<int>& membersId);
    std::optional<QList<int>> getMembersOfChat(int chatId);
    QList<Chat> getChatsOfUser(int id);
    int getMembersCount(int chat_id);
    OptionalUserId getOtherMemberId(int chat_id, int userId);
    OptionalChat getChatById(int chatId);

private:
    QSqlDatabase getThreadDatabase();
    template<typename... Args>
    bool executeQuery(QSqlQuery& query, Args&&... args);
    Chat getChatFromQuery(QSqlQuery& query, int chatId);
};

#endif // DATABASE_H
