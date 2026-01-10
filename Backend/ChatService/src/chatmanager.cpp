#include "chatservice/chatmanager.h"

#include "Fields.h"
#include "entities/ChatMember.h"
#include "entities/PrivateChat.h"
#include "interfaces/IIdGenerator.h"

namespace {

bool checkIdValid(long long id) { return id > 0; }

}  // namespace

ChatManager::ChatManager(GenericRepository *repository, IIdGenerator *generator)
    : repository_(repository), generator_(generator) {}

std::optional<PrivateChat> ChatManager::getPrivateChat(ID first_user_id, ID second_user_id) {
  if (first_user_id == second_user_id) {
    LOG_ERROR("Invalid sitution with ID");
    return std::nullopt;
  }

  if (first_user_id > second_user_id) std::swap(first_user_id, second_user_id);

  auto custom_query = QueryFactory::createSelect<PrivateChat>(repository_->getExecutor(), repository_->getCache());
  custom_query->where(PrivateChatTable::FirstUserId, first_user_id)
      .where(PrivateChatTable::SecondUserId, second_user_id);
  auto result = custom_query->execute();

  if (auto res = QueryFactory::getSelectResult(result).result; res.size() == 1) {
    LOG_INFO("Private chat is existed, id is {}", res[0].chat_id);
    return res[0];
  }

  return std::nullopt;
}

std::optional<ID> ChatManager::createPrivateChat(ID first_user_id, ID second_user_id) {
  LOG_INFO("F id {} and S id {}", first_user_id, second_user_id);
  if (first_user_id == second_user_id) {
    LOG_ERROR("User id is same");
    return std::nullopt;
  }

  if (!checkIdValid(first_user_id)) {
    LOG_ERROR("User first_user_id is invalid");
    return std::nullopt;
  }

  if (!checkIdValid(second_user_id)) {
    LOG_ERROR("User second_user_id is invalid");
    return std::nullopt;
  }

  long long min_user_id = first_user_id < second_user_id ? first_user_id : second_user_id;
  long long max_user_id = first_user_id > second_user_id ? first_user_id : second_user_id;

  if (auto existed_chat = getPrivateChat(min_user_id, max_user_id); existed_chat.has_value()) {
    LOG_INFO("Private chat is existed, id is {}", existed_chat->chat_id);
    return existed_chat->chat_id;
  }

  long long new_chat_id = generator_->generateId();
  if (!checkIdValid(new_chat_id)) {
    LOG_ERROR("INvalid generated id");
    return std::nullopt;
  }

  LOG_INFO("Create chat with id {}", new_chat_id);
  Chat to_save;
  to_save.id = new_chat_id;
  to_save.created_at = QDateTime::currentSecsSinceEpoch();
  to_save.is_group = false;

  if (!repository_->save(to_save)) {
    LOG_ERROR("Can't save chat in db");
    return std::nullopt;
  }

  PrivateChat private_chat;
  private_chat.chat_id = to_save.id;
  private_chat.first_user = min_user_id;
  private_chat.second_user = max_user_id;

  if (!repository_->save(private_chat)) {
    LOG_ERROR("Can't save private chat");
    return std::nullopt;
  }

  if (!addMembersToChat(to_save.id, {min_user_id, max_user_id})) {
    LOG_ERROR("Can't save members");
    return std::nullopt;
  }

  return to_save.id;
}

bool ChatManager::addMembersToChat(ID chat_id, const std::vector<ID> &members_id) {
  if (!checkIdValid(chat_id)) {
    LOG_ERROR("Invalid chat_id in addMembersToChat {}", chat_id);
  }
  std::vector<ChatMember> chat_members;
  for (auto id : members_id) {
    if (!checkIdValid(id)) {
      LOG_ERROR("Invalid member id");
      continue;
    }
    LOG_INFO("Add member with id {} to chat {}", id, chat_id);
    ChatMember new_member(chat_id, id, QDateTime::currentSecsSinceEpoch());
    chat_members.push_back(new_member);
  }

  for (auto chat_member : chat_members) {
    bool ok = repository_->save(chat_member);
    if (!ok) {
      LOG_INFO("Member {} for chat {} is not saved", chat_member.user_id, chat_member.chat_id);
      return false;
    }
  }
  // TODO: repository_->save(chat_members);
  return true;
}

std::vector<ID> ChatManager::getMembersOfChat(ID chat_id) {
  if (!checkIdValid(chat_id)) {
    LOG_ERROR("In getMembersOfChat {} is invalid", chat_id);
    return std::vector<ID>{};
  }
  auto chat_members = repository_->findByField<ChatMember>(ChatMemberTable::ChatId, chat_id);
  // TODO: make select only id of chat_members
  LOG_INFO("[getMembersOfChat] for chat_id {} finded {} members", chat_id, chat_members.size());
  std::vector<ID> chat_members_id;
  for (auto member : chat_members) {
    if (!checkIdValid(member.user_id)) {
      LOG_ERROR("Chat member with invalid id: {}", nlohmann::json(member).dump());
    } else {
      LOG_INFO("[getMembersOfChat] for chat_id {} finded {} ", chat_id, nlohmann::json(member).dump());
      chat_members_id.push_back(member.user_id);
    }
  }
  return chat_members_id;
}

std::vector<ID> ChatManager::getChatsIdOfUser(ID user_id) {
  LOG_INFO("getChatsIdOfUser {}", user_id);
  auto chats_where_user_member = repository_->findByField<ChatMember>(ChatMemberTable::UserId, user_id);
  LOG_INFO("chats_where_user {} is member is {} size", user_id, (int)chats_where_user_member.size());

  std::vector<ID> ids;
  for (const auto &member : chats_where_user_member) ids.push_back(member.chat_id);

  return ids;
}

int ChatManager::getMembersCount(ID chat_id) {
  auto members_id = getMembersOfChat(chat_id);
  return members_id.size();
}

std::optional<ID> ChatManager::getOtherMemberId(ID chat_id, ID user_id) {
  if (!checkIdValid(chat_id)) {
    LOG_ERROR("Invalid chat_id in getOtherMemberId {}", chat_id);
    return std::nullopt;
  }

  if (!checkIdValid(user_id)) {  // todo: struct ID that can't be <= 0
    LOG_ERROR("Invalid user_id in getOtherMemberId {}", chat_id);
    return std::nullopt;
  }

  auto members_id = getMembersOfChat(chat_id);
  for (auto mem_id : members_id) {
    if (mem_id != user_id) return mem_id;
  }
  return std::nullopt;
}

std::optional<Chat> ChatManager::getChatById(ID chat_id) { return repository_->findOne<Chat>(chat_id); }
