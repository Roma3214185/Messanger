#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QList>
#include <optional>
#include <vector>

#include "GenericRepository.h"
#include "entities/Chat.h"
#include "chatservice/interfaces/IChatManager.h"

class ChatManager : public IChatManager {
 public:
  ChatManager(GenericRepository* repository);
  std::optional<Chat> createPrivateChat(int first_member, int second_user) override;
  bool                addMembersToChat(int chat_id, const std::vector<int>& members_id) override;
  std::vector<int>    getMembersOfChat(int chat_id) override;
  std::vector<Chat>   getChatsOfUser(int user_id) override;
  int                 getMembersCount(int chat_id) override;
  std::optional<int>  getOtherMemberId(int chat_id, int user_id) override;
  std::optional<Chat> getChatById(int chat_id) override;

 private:
  GenericRepository* repository_;
};

#endif  // CHATMANAGER_H
