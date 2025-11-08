#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <optional>
#include <vector>
#include <QList>

#include "Persistence/GenericRepository.h"
#include "entities/Chat.h"

class ChatManager {
  public:
    ChatManager(GenericRepository* repository);

    std::optional<int> createPrivateChat();
    bool addMembersToChat(int chat_id, const std::vector<int>& members_id);
    //bool deleteChat(int chat_id);
    //bool deleteMembersFromChat(int chatId, const std::vector<int>& members_id);
    std::vector<int> getMembersOfChat(int chat_id);
    std::vector<Chat> getChatsOfUser(int user_id);
    int getMembersCount(int chat_id);
    std::optional<int> getOtherMemberId(int chat_id, int user_id);
    std::optional<Chat> getChatById(int chat_id);
  private:
    GenericRepository* repository_;
};

#endif // CHATMANAGER_H
