#include "managers/datamanager.h"

#include "Debug_profiling.h"
#include "entities/MessageStatus.h"
#include "utils.h"

namespace {

void decrease(std::unordered_map<long long, int> &reactions, std::optional<long long> reaction_id) {
  if(!reaction_id.has_value()) return;
  reactions[*reaction_id]--;
  if (reactions[*reaction_id] <= 0) reactions.erase(*reaction_id);
}

bool isMyReaction(const Message &message, const Reaction &reaction) {
  return message.receiver_id == reaction.receiver_id;
}

}  // namespace

auto DataManager::getIterMessageById(long long message_id) {
  DBC_REQUIRE(message_id > 0);
  return std::ranges::find_if(messages_,
                              [&](const auto &existing_message) { return existing_message.id == message_id; });
}

auto DataManager::getIterMessageByLocalId(const QString &local_id) {
  DBC_REQUIRE(!local_id.isEmpty());
  return std::ranges::find_if(messages_,
                              [&](const auto &existing_message) { return existing_message.local_id == local_id; });
}

ChatPtr DataManager::getPrivateChatWithUser(long long user_id) {
  for (auto &[_, chat] : chats_by_id_) {
    if (chat->isPrivate()) {
      auto *pchat = static_cast<PrivateChat *>(chat.get());
      if (pchat->user_id == user_id) {
        LOG_INFO("Found private chat for this user '{}' and id '{}'", pchat->title.toStdString(), pchat->chat_id);
        return chat;
      }
    }
  }
  return nullptr;
}

MessageModelPtr DataManager::getMessageModel(long long chat_id) {
  auto iter = message_models_by_chat_id_.find(chat_id);
  if (iter != message_models_by_chat_id_.end() && iter->second != nullptr) return iter->second;

  auto message_model = std::make_shared<MessageModel>();
  message_models_by_chat_id_[chat_id] = message_model;
  return message_model;
}

ChatPtr DataManager::getChat(long long chat_id) {
  auto chat_iter = chats_by_id_.find(chat_id);
  return chat_iter != chats_by_id_.end() ? chat_iter->second : nullptr;
}

int DataManager::getNumberOfExistingChats() const noexcept { return static_cast<int>(chats_by_id_.size()); }

int DataManager::getNumberOfExistingUsers() const noexcept { return static_cast<int>(users_by_id_.size()); }

void DataManager::clearAllChats() { chats_by_id_.clear(); }

void DataManager::clearAllMessageModels() { message_models_by_chat_id_.clear(); }

void DataManager::clearAllUsers() { users_by_id_.clear(); }

void DataManager::clearAll() {
  clearAllUsers();
  clearAllMessageModels();
  clearAllChats();
}

void DataManager::save(const ChatPtr &chat, MessageModelPtr message_model) {
  DBC_REQUIRE(chat->chat_id > 0);
  save(chat->default_reactions);
  if (!message_model) message_model = std::make_shared<MessageModel>();

  const std::scoped_lock lock(chat_mutex_);
  bool chat_was = chats_by_id_.contains(chat->chat_id);
  chats_by_id_[chat->chat_id] = chat;
  message_models_by_chat_id_[chat->chat_id] = message_model;
  if (!chat_was) Q_EMIT chatAdded(chat);
}

void DataManager::save(const User &user) {
  DBC_REQUIRE(user.checkInvariants());
  const std::scoped_lock lock(user_mutex_);
  users_by_id_[user.id] = user;
}

std::optional<User> DataManager::getUser(UserId user_id) {
  DBC_REQUIRE(user_id > 0);
  const std::scoped_lock lock(user_mutex_);
  auto it = users_by_id_.find(user_id);
  return it == users_by_id_.end() ? std::nullopt : std::make_optional(it->second);
}

std::optional<Message> DataManager::getMessageById(const long long message_id) {
  DBC_REQUIRE(message_id > 0);
  auto it = getIterMessageById(message_id);
  return it == messages_.end() ? std::nullopt : std::make_optional(*it);
}

void DataManager::save(const Message &message) {
  const std::scoped_lock lock(messages_mutex_);
  auto it = getIterMessageByLocalId(message.local_id);
  LOG_INFO("To Save message {}", message.toString());

  if (it != messages_.end()) {
    LOG_INFO("Message already exist");
    DBC_REQUIRE(it->id == 0 || it->id == message.id);
    it->updateFrom(message);
  } else {
    LOG_INFO("Message will be added");
    messages_.push_back(message);
  }

  Q_EMIT messageAdded(message);  // todo: messageAdded :->: messageSaved() ->
                                 // updateChatIconInList
}

void DataManager::deleteMessage(const Message &msg) {
  const std::scoped_lock lock(messages_mutex_);
  auto it = getIterMessageByLocalId(msg.local_id);
  if (it == messages_.end()) {
    LOG_WARN("Message {} to delete not found", msg.toString());
    return;
  }

  LOG_INFO("Delete message {}", msg.toString());
  messages_.erase(it);
  Q_EMIT messageDeleted(msg);
}
void DataManager::save(const MessageStatus &message_status) {
  std::scoped_lock lock(messages_mutex_);
  DBC_REQUIRE(message_status.checkInvariants());
  DBC_REQUIRE(message_status.is_read);

  auto it = getIterMessageById(message_status.message_id);
  if (it == messages_.end()) { // can be if u delete message for yourself
    LOG_WARN("To read message with id {} not found", message_status.message_id);
  } else if (it->receiver_read_status) { // sometimes will be because i firstly save offline
    LOG_INFO("{} is already marked readed", it->toString());
  } else {
    it->read_counter++;
    it->receiver_read_status = true;
    Q_EMIT messageAdded(*it); //todo: messageChanged
  }
}

void DataManager::save(const ReactionInfo &reaction_info) {
  reactions_[reaction_info.id] = reaction_info;  // todo: map of locks
}

std::optional<ReactionInfo> DataManager::getReactionInfo(long long reaction_id) {
  DBC_REQUIRE(reaction_id > 0);
  auto it = reactions_.find(reaction_id);
  return it != reactions_.end() ? std::make_optional(it->second) : std::nullopt;
}

void DataManager::deleteReaction(const Reaction &reaction_to_delete) {
  std::scoped_lock lock(messages_mutex_);
  DBC_REQUIRE(reaction_to_delete.checkInvariants());
  auto iter_on_message = getIterMessageById(reaction_to_delete.message_id);
  if (iter_on_message == messages_.end()) {
    LOG_INFO("To delete reaction message not found");
    return;
  }

  // todo: lock mutex for this message: message_mutexes_by_id_[message.id].lock();
  Message &message_to_delete_reaction = *iter_on_message;
  bool my_reaction = isMyReaction(message_to_delete_reaction, reaction_to_delete);

  if (my_reaction && !utils::isSame(message_to_delete_reaction.receiver_reaction, reaction_to_delete.reaction_id)) {
    LOG_INFO("It's my reaction, and {} reaction already was deleted", reaction_to_delete.reaction_id);
    return;
  }

  decrease(message_to_delete_reaction.reactions, reaction_to_delete.reaction_id);

  if (my_reaction) {
    message_to_delete_reaction.receiver_reaction.reset();
  }
  Q_EMIT messageAdded(message_to_delete_reaction);  // todo: messageChanged
}

void DataManager::save(const Reaction &reaction_to_save) {
  std::scoped_lock lock(messages_mutex_);
  DBC_REQUIRE(reaction_to_save.checkInvariants());
  auto iter_on_message = getIterMessageById(reaction_to_save.message_id);
  if (iter_on_message == messages_.end()) {
    DBC_UNREACHABLE();
    return;
  }

  Message &message_to_save_reaction = *iter_on_message;
  bool my_reaction = isMyReaction(message_to_save_reaction, reaction_to_save);

  if (my_reaction && message_to_save_reaction.receiver_reaction == reaction_to_save.reaction_id) {
    LOG_INFO("It's my reaction, and it exists already");
    return;
  }

  // todo: lock mutex for this message: message_mutexes_by_id_[message.id].lock();
  message_to_save_reaction.reactions[reaction_to_save.reaction_id]++;

  if (my_reaction) { // it's my reaction, so i need to delete my old reaction if it setted, and set new one
    decrease(message_to_save_reaction.reactions, message_to_save_reaction.receiver_reaction);
    message_to_save_reaction.receiver_reaction = reaction_to_save.reaction_id;
  }

  Q_EMIT messageAdded(message_to_save_reaction);  // todo: messageChanged
}

std::vector<ReactionInfo> DataManager::getEmojiesForMenu() {
  // temporarely: return all available emojies, todo: implement request on server for getting them first
  std::vector<ReactionInfo> emojies;
  for (auto &[id, emoji] : reactions_) emojies.push_back(emoji);
  return emojies;
}
