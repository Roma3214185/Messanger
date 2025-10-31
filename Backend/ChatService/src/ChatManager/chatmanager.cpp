#include "chatmanager.h"

#include "headers/ChatMember.h"

ChatManager::ChatManager(GenericRepository* repository)
    : repository_(repository) { }

std::optional<int> ChatManager::createPrivateChat(){
  Chat new_chat;
  new_chat.is_group = false;
  new_chat.id = 0;
  bool ok = repository_->save(new_chat);
  if(!ok) return std::nullopt;
  LOG_INFO("Private chat is created with id {}", new_chat.id);
  return new_chat.id;
}

bool ChatManager::addMembersToChat(int chat_id, const std::vector<int>& members_id){
  std::vector<ChatMember> chat_members;
  for(auto id: members_id){
    LOG_INFO("Add member with id {} to chat {}", id, chat_id);
    ChatMember new_member{
      .id = chat_id,
      .user_id = id,
      .added_at = QDateTime::currentSecsSinceEpoch()
    };
    chat_members.push_back(new_member);
  }

  for(auto chat_member: chat_members){
    bool ok = repository_->save(chat_member);
    LOG_INFO("Member {} for chat {} is saved {}", chat_member.user_id,
             chat_member.id, ok);
  }
  //repository_->save(chat_members);
  return true;
}

//bool deleteChat(int chat_id);
//bool deleteMembersFromChat(int chatId, const std::vector<int>& members_id);

std::vector<int> ChatManager::getMembersOfChat(int chat_id){
  std::vector<ChatMember> chat_members = repository_->findByField<ChatMember>("id", chat_id);
  LOG_INFO("[getMembersOfChat] for chat_id {} finded {} members", chat_id, chat_members.size());
  std::vector<int> chat_members_id;
  for(auto member: chat_members){
    LOG_INFO("[getMembersOfChat] for chat_id {} finded {} ", chat_id, member.user_id);
    chat_members_id.push_back(member.user_id);
  }
  return chat_members_id;
}

std::vector<Chat> ChatManager::getChatsOfUser(int user_id){
  auto chats_where_user_member =  repository_->findByField<ChatMember>("user_id", user_id);
  std::vector<Chat> chats_of_user;
  for(auto chat_member: chats_where_user_member){
    int chat_id = chat_member.id;
    auto chat = repository_->findOne<Chat>(chat_id);
    if(chat) chats_of_user.push_back(*chat);
    else LOG_ERROR("Could find chat with id {}", chat_id);
  }
  return chats_of_user;
  // TODO(roma): make this func using join
}

int ChatManager::getMembersCount(int chat_id){
  auto members_id = getMembersOfChat(chat_id);
  return members_id.size();
}

std::optional<int> ChatManager::getOtherMemberId(int chat_id, int user_id){
  auto members_id = getMembersOfChat(chat_id);
  for(auto mem_id: members_id){
    if(mem_id != user_id) return mem_id;
  }
  return std::nullopt;
}

std::optional<Chat> ChatManager::getChatById(int chat_id){
  repository_->findOne<Chat>(chat_id);
}
