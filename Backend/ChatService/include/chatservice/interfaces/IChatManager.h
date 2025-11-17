#ifndef ICHATMANAGER_H
#define ICHATMANAGER_H

#include <vector>
#include <optional>
#include <entities/Chat.h>

class IChatManager {
  public:
    virtual ~IChatManager() = default;
    virtual std::optional<Chat> createPrivateChat(int first_member, int second_member) = 0;
    virtual bool                addMembersToChat(int chat_id, const std::vector<int>& members_id) = 0;
    virtual std::vector<int>    getMembersOfChat(int chat_id) = 0;
    virtual std::vector<Chat>   getChatsOfUser(int user_id) = 0;
    virtual int                 getMembersCount(int chat_id) = 0;
    virtual std::optional<int>  getOtherMemberId(int chat_id, int user_id) = 0;
    virtual std::optional<Chat> getChatById(int chat_id) = 0;
};

#endif // ICHATMANAGER_H
