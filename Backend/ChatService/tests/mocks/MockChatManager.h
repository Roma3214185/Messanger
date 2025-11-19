#ifndef MOCKCHATMANAGER_H
#define MOCKCHATMANAGER_H

#include <chatservice/interfaces/IChatManager.h>

class MockChatManager : public IChatManager {
  public:
    int last_chat_id;
    std::optional<Chat> mock_chat;
    std::optional<ID> mock_chat_id;
    std::optional<ID> mock_other_member_id;
    int mock_cht;
    std::vector<Chat> mock_chats;
    std::vector<ID> mock_members;
    std::vector<ID> mock_chat_ids;
    int last_user_id;

    int call_createPrivateChat = 0;
    int call_addMembersToChat = 0;
    int call_getMembersOfChat = 0;
    int call_getChatsOfUser = 0;
    int call_getMembersCount = 0;
    int call_getOtherMemberId = 0;
    int call_getChatById = 0;
    std::pair<ID, ID> last_createPrivateChat;

    std::unordered_map<ID, std::optional<Chat>> mock_chat_by_id;

    std::optional<ID> createPrivateChat(ID first_member, ID second_member) override {
      ++call_createPrivateChat;
      last_createPrivateChat = std::make_pair(first_member, second_member);
      return mock_chat_id;
    }

    bool addMembersToChat(ID chat_id, const std::vector<ID>& members_id) override {
      last_chat_id = chat_id;
      ++call_addMembersToChat;
    }

    std::vector<ID> getMembersOfChat(ID chat_id) override {
      ++call_getMembersOfChat;
      last_chat_id = chat_id;
      return mock_members;
    }

    std::vector<ID> getChatsIdOfUser(ID user_id) override {
      last_user_id = user_id;
      ++call_getChatsOfUser;
      return mock_chat_ids;
    }

    int getMembersCount(ID chat_id) override {
      ++call_getMembersCount;
      last_chat_id = chat_id;
      return mock_cht;
    }

    std::optional<ID> getOtherMemberId(ID chat_id, ID user_id) override {
      ++call_getOtherMemberId;
      return mock_other_member_id;
    }

    std::optional<Chat> getChatById(ID chat_id) override {
      ++call_getChatById;
      last_chat_id = chat_id;
      if (mock_chat_by_id.find(chat_id) != mock_chat_by_id.end()) {
        return mock_chat_by_id[chat_id];
      }
      return std::nullopt;
    }

    void reset() {
      mock_chat.reset();
      mock_chat_id.reset();
      mock_other_member_id.reset();
      mock_cht = 0;
      mock_chats.clear();
      mock_members.clear();
      mock_chat_ids.clear();
      call_createPrivateChat = 0;
      call_addMembersToChat = 0;
      call_getMembersOfChat = 0;
      call_getChatsOfUser = 0;
      call_getMembersCount = 0;
      call_getOtherMemberId = 0;
      call_getChatById = 0;
      mock_chat_by_id.clear();
    }
};

#endif // MOCKCHATMANAGER_H
