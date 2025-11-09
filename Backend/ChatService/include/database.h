#ifndef BACKEND_CHATSERVICE_SRC_DATABASE_DATABASE_H_
#define BACKEND_CHATSERVICE_SRC_DATABASE_DATABASE_H_

#include <QList>
#include <string>
#include <vector>

#include "GenericRepository.h"
#include "entities/Chat.h"

using ChatId         = int;
using UserId         = int;
using OptionalChatId = std::optional<int>;
using OptionalChat   = std::optional<Chat>;
using OptionalUserId = std::optional<UserId>;

class DataBase {
 public:
  void                      clearDataBase();
  OptionalChatId            createPrivateChat();
  bool                      addMembersToChat(int chat_id, const std::vector<int>& members_id);
  bool                      initialDb();
  bool                      deleteChat(int chat_id);
  bool                      deleteMembersFromChat(int chatId, const std::vector<int>& members_id);
  std::optional<QList<int>> getMembersOfChat(int chat_id);
  QList<Chat>               getChatsOfUser(int chat_id);
  int                       getMembersCount(int chat_id);
  OptionalUserId            getOtherMemberId(int chat_id, int user_id);
  OptionalChat              getChatById(int chat_id);

 private:
  QSqlDatabase getThreadDatabase();
  template <typename... Args>
  bool executeQuery(QSqlQuery& query, Args&&... args);
  Chat getChatFromQuery(QSqlQuery& query, int chat_id);
};

#endif  // BACKEND_CHATSERVICE_SRC_DATABASE_DATABASE_H_
