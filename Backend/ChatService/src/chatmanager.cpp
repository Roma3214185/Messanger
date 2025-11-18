#include "chatservice/chatmanager.h"

#include "entities/ChatMember.h"

ChatManager::ChatManager(GenericRepository* repository) : repository_(repository) {}

std::optional<ID> ChatManager::createPrivateChat(ID first_user_id, ID second_user_id) {
  // PrivateChat new_chat; //
  // new_chat.is_group = false;
  // new_chat.id       = 0;
  // bool ok           = repository_->save(new_chat);
  // if (!ok) return std::nullopt;
  // LOG_INFO("Private chat is created with id {}", new_chat.id);
  // return new_chat;
}

bool ChatManager::addMembersToChat(ID chat_id, const std::vector<ID>& members_id) {
  std::vector<ChatMember> chat_members;
  for (auto id : members_id) {
    LOG_INFO("Add member with id {} to chat {}", id, chat_id);
    ChatMember new_member{
        .chat_id = chat_id, .user_id = id, .added_at = QDateTime::currentSecsSinceEpoch()};
    chat_members.push_back(new_member);
  }

  for (auto chat_member : chat_members) {
    bool ok = repository_->save(chat_member);
    LOG_INFO("Member {} for chat {} is saved {}", chat_member.user_id, chat_member.chat_id, ok);
  }
  // repository_->save(chat_members);
  return true;
}

// bool deleteChat(int chat_id);
// bool deleteMembersFromChat(int chatId, const std::vector<int>& members_id);

std::vector<ID> ChatManager::getMembersOfChat(ID chat_id) {
  std::vector<ChatMember> chat_members = repository_->findByField<ChatMember>("chat_id", chat_id);
  LOG_INFO("[getMembersOfChat] for chat_id {} finded {} members", chat_id, chat_members.size());
  std::vector<ID> chat_members_id;
  for (auto member : chat_members) {
    LOG_INFO("[getMembersOfChat] for chat_id {} finded {} ", chat_id, member.user_id);
    chat_members_id.push_back(member.user_id);
  }
  return chat_members_id;
}

std::vector<ID> ChatManager::getChatsIdOfUser(ID user_id) {
  // LOG_INFO("[TEMP] Trt get chats of user {}", user_id);
  // auto chats_where_user_member = repository_->findByField<ChatMember>("user_id", user_id);
  // LOG_INFO("[TEMP] Get chats of user {}: size {}", user_id, chats_where_user_member.size());
  // std::vector<Chat> chats_of_user;
  // for (auto chat_member : chats_where_user_member) {
  //   int  chat_id = chat_member.chat_id;
  //   auto chat    = repository_->findOne<Chat>(chat_id);
  //   if (chat)
  //     chats_of_user.push_back(*chat);
  //   else
  //     LOG_ERROR("Could find chat with id {}", chat_id);
  // }
  // return chats_of_user;
  // // TODO(roma): make this func using join
}

int ChatManager::getMembersCount(ID chat_id) {
  auto members_id = getMembersOfChat(chat_id);
  return members_id.size();
}

std::optional<ID> ChatManager::getOtherMemberId(ID chat_id, ID user_id) {
  auto members_id = getMembersOfChat(chat_id);
  for (auto mem_id : members_id) {
    if (mem_id != user_id) return mem_id;
  }
  return std::nullopt;
}

std::optional<Chat> ChatManager::getChatById(ID chat_id) {
  return repository_->findOne<Chat>(chat_id);
}
