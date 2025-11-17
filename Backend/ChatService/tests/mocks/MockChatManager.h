#ifndef MOCKCHATMANAGER_H
#define MOCKCHATMANAGER_H

#include <chatservice/interfaces/IChatManager.h>

class MockChatManager : public IChatManager {
  public:
    int last_chat_id;
    std::optional<Chat> mock_chat;
    std::optional<int> mock_chat_id;
    std::optional<int> mock_other_member_id;
    int mock_cht;
    std::vector<Chat> mock_chats;
    std::vector<int> mock_members;
    int last_user_id;

    int call_createPrivateChat = 0;
    int call_addMembersToChat = 0;
    int call_getMembersOfChat = 0;
    int call_getChatsOfUser = 0;
    int call_getMembersCount = 0;
    int call_getOtherMemberId = 0;
    int call_getChatById = 0;
    std::pair<int, int> last_createPrivateChat;


    std::optional<int> createPrivateChat(int first_member, int second_member) {
      ++call_createPrivateChat;
      last_createPrivateChat = std::make_pair(first_member, second_member);
      return mock_chat_id;
    }

    bool addMembersToChat(int chat_id, const std::vector<int>& members_id) {
      ++call_addMembersToChat;
      last_chat_id = chat_id;
    }

    std::vector<int> getMembersOfChat(int chat_id) {
      LOG_INFO("[temp] i in getMembersOfChat {}", chat_id);
      ++call_getMembersOfChat;
      last_chat_id = chat_id;
      return mock_members;
    }

    std::vector<Chat> getChatsOfUser(int user_id) {
      last_user_id = user_id;
      ++call_getChatsOfUser;
      return mock_chats;
    }

    int getMembersCount(int chat_id) {
      ++call_getMembersCount;
      last_chat_id = chat_id;
      return mock_cht;
    }

    std::optional<int> getOtherMemberId(int chat_id, int user_id) {
      ++call_getOtherMemberId;
      return mock_other_member_id;
    }

    std::optional<Chat> getChatById(int chat_id) {
      last_chat_id = chat_id;
      ++call_getChatById;
      return mock_chat;
    }
};

#endif // MOCKCHATMANAGER_H
