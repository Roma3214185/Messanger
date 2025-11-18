#ifndef ICHATMANAGER_H
#define ICHATMANAGER_H

#include <vector>
#include <optional>
#include <entities/Chat.h>

using ID = long long;

class IChatManager {
  public:
    virtual ~IChatManager() = default;
    virtual std::optional<ID> createPrivateChat(ID first_member, ID second_member) = 0;
    virtual bool                addMembersToChat(ID chat_id, const std::vector<ID>& members_id) = 0;
    virtual std::vector<ID>    getMembersOfChat(ID chat_id) = 0;
    virtual std::vector<ID>    getChatsIdOfUser(ID user_id) = 0;
    virtual int                 getMembersCount(ID chat_id) = 0;
    virtual std::optional<ID>  getOtherMemberId(ID chat_id, ID user_id) = 0;
    virtual std::optional<Chat> getChatById(ID chat_id) = 0;
};

#endif // ICHATMANAGER_H
