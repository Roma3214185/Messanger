#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QList>
#include <optional>
#include <vector>

#include "GenericRepository.h"
#include "entities/Chat.h"
#include "chatservice/interfaces/IChatManager.h"

class IIdGenerator;

using ID = long long;

class ChatManager : public IChatManager {
 public:
  ChatManager(GenericRepository* repository, IIdGenerator* generator);
  std::optional<ID> createPrivateChat(ID first_member, ID second_user) override;
  bool                addMembersToChat(ID chat_id, const std::vector<ID>& members_id) override;
  std::vector<ID>    getMembersOfChat(ID chat_id) override;
  std::vector<ID>    getChatsIdOfUser(ID user_id) override;
  int                 getMembersCount(ID chat_id) override;
  std::optional<ID>  getOtherMemberId(ID chat_id, ID user_id) override;
  std::optional<Chat> getChatById(ID chat_id) override;

 private:
  GenericRepository* repository_;
  IIdGenerator* generator_;
};

#endif  // CHATMANAGER_H
