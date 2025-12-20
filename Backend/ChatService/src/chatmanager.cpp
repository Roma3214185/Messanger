#include "chatservice/chatmanager.h"
#include "entities/PrivateChat.h"
#include "entities/ChatMember.h"
#include "Fields.h"
#include "interfaces/IIdGenerator.h"

ChatManager::ChatManager(GenericRepository* repository, IIdGenerator* generator)
    : repository_(repository), generator_(generator) {}

std::optional<ID> ChatManager::createPrivateChat(ID first_user_id, ID second_user_id) {
  // 1) check in db "private_chats" exists of pair min(first_user_id, second_user) and max(first_user_id, second_user)
  // 2) if chat exist -> return id
  // 3) else (returned empty list)
  // 4) create in db "chat" chat with is_group = false
  // 5) if success -> get id and save in db
            //"private_chats" (chat_id, min(first_user_id, second_user) max(first_user_id, second_user)
  //6) save in chat_members pair chat_id, first_user_id
  //7) save in chat_members pair chat_id, second_user_id

  if(first_user_id == second_user_id) {
    LOG_ERROR("User id is same");
    return std::nullopt;
  }

  int min_user_id = first_user_id < second_user_id ? first_user_id : second_user_id;
  int max_user_id = first_user_id > second_user_id ? first_user_id : second_user_id;

  auto custom_query = QueryFactory::createSelect<PrivateChat>(repository_->getExecutor(), repository_->getCache());
  custom_query->where(PrivateChatTable::FirstUserId, min_user_id)
      .where(PrivateChatTable::SecondUserId, max_user_id);

  auto res = custom_query->execute();
  assert(res.size() <= 1);
  if(res.size() == 1) return res[0].chat_id;

  int new_chat_id = generator_->generateId();
  Chat to_save;
  to_save.id = new_chat_id;
  to_save.created_at = QDateTime::currentSecsSinceEpoch();
  to_save.is_group = false;

  bool ok = repository_->save(to_save);
  if(!ok) {
    LOG_ERROR("Can't save chat in db");
    return std::nullopt;
  }

  PrivateChat private_chat;
  private_chat.chat_id = to_save.id;
  private_chat.first_user = min_user_id;
  private_chat.second_user = max_user_id;

  bool save_private_chat = repository_->save(private_chat);
  if(!save_private_chat) {
    LOG_ERROR("Can't save private chat");
    return std::nullopt;
  }

  bool save_members = addMembersToChat(to_save.id, {min_user_id, max_user_id});
  if(!save_members) {
    LOG_ERROR("Can't save members");
    return std::nullopt;
  }

  return to_save.id;
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
    if(!ok) {
      LOG_INFO("Member {} for chat {} is not saved", chat_member.user_id, chat_member.chat_id);
      return false;
    }
  }
  // TODO: repository_->save(chat_members);
  return true;
}

std::vector<ID> ChatManager::getMembersOfChat(ID chat_id) {
  std::vector<ChatMember> chat_members = repository_->findByField<ChatMember>(ChatMemberTable::ChatId, chat_id);
  LOG_INFO("[getMembersOfChat] for chat_id {} finded {} members", chat_id, chat_members.size());
  std::vector<ID> chat_members_id;
  for (auto member : chat_members) {
    LOG_INFO("[getMembersOfChat] for chat_id {} finded {} ", chat_id, member.user_id);
    chat_members_id.push_back(member.user_id);
  }
  return chat_members_id;
}

std::vector<ID> ChatManager::getChatsIdOfUser(ID user_id) {
  auto chats_where_user_member = repository_->findByField<ChatMember>(ChatMemberTable::UserId, user_id);
  LOG_INFO("chats_where_user {} is member is {} size", user_id, (int)chats_where_user_member.size());

  std::vector<ID> ids;
  for(const auto& member: chats_where_user_member) ids.push_back(member.chat_id);

  return ids;
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
