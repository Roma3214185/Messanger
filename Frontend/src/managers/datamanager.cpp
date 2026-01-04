#include "managers/datamanager.h"

#include "Debug_profiling.h"

ChatPtr DataManager::getPrivateChatWithUser(long long user_id) {
  for (auto [_, chat] : chats_by_id_) {
    if (chat->isPrivate()) {
      auto* pchat = static_cast<PrivateChat*>(chat.get());
      if (pchat->user_id == user_id) {
        LOG_INFO("Found private chat for this user '{}' and id '{}'",
                 pchat->title.toStdString(),
                 pchat->chat_id);
        return chat;
      }
    }
  }
  return nullptr;
}

MessageModelPtr DataManager::getMessageModel(long long chat_id) {
  auto iter = message_models_by_chat_id_.find(chat_id);
  if (iter != message_models_by_chat_id_.end()) return iter->second;

  auto message_model                  = std::make_shared<MessageModel>();
  message_models_by_chat_id_[chat_id] = message_model;
  return message_model;
}

ChatPtr DataManager::getChat(long long chat_id) {
  auto chat_iter = chats_by_id_.find(chat_id);
  if (chat_iter == chats_by_id_.end())
    return nullptr;  // todo: maybe return empty chat but then load all messages??
  return chat_iter->second;
}

int DataManager::getNumberOfExistingChats() const noexcept { return chats_by_id_.size(); }

int DataManager::getNumberOfExistingUsers() const noexcept { return users_by_id_.size(); }

void DataManager::clearAllChats() { chats_by_id_.clear(); }

void DataManager::clearAllMessageModels() { message_models_by_chat_id_.clear(); }

void DataManager::clearAllUsers() { users_by_id_.clear(); }

void DataManager::clearAll() {
  clearAllUsers();
  clearAllMessageModels();
  clearAllChats();
}

// void DataManager::deleteChat(long long id) {
//   if(id <= 0) return LOG_WARN("Invalid id to delete chat");
//   auto it = chats_by_id_.find(id);
//   if(it != chats_by_id_.end()) {
//      Q_EMIT chatDeleted(it->second);
//      chats_by_id_.erase(chat->chat_id);
//      message_models_by_chat_id_.erase(chat->chat_id);
//    } else {
//      LOG_WARN();
//    }
// }

void DataManager::addChat(ChatPtr chat, MessageModelPtr message_model) {
  DBC_REQUIRE(chat->chat_id > 0);
  if (!message_model) message_model = std::make_shared<MessageModel>();

  const std::lock_guard<std::mutex> lock(chat_mutex_);
  if (!chats_by_id_.contains(chat->chat_id)) Q_EMIT chatAdded(chat);
  chats_by_id_[chat->chat_id]               = chat;
  message_models_by_chat_id_[chat->chat_id] = message_model;
}

void DataManager::saveUser(const User& user) {
  DBC_REQUIRE(user.checkInvariants());
  const std::lock_guard<std::mutex> lock(user_mutex_);
  users_by_id_[user.id] = user;
}

std::optional<User> DataManager::getUser(UserId user_id) {
  DBC_REQUIRE(user_id > 0);
  const std::lock_guard<std::mutex> lock(user_mutex_);
  auto                              it = users_by_id_.find(user_id);
  return it == users_by_id_.end() ? std::nullopt : std::make_optional(it->second);
}

std::optional<Message> DataManager::getMessageById(const long long message_id) {
  DBC_REQUIRE(message_id > 0);
  auto it = std::find_if(messages_.begin(), messages_.end(), [&](const auto& message) {
    return message.id == message_id;
  });
  return it == messages_.end() ? std::nullopt : std::make_optional(*it);
}

void DataManager::saveMessage(const Message& message) {
  const std::lock_guard<std::mutex> lock(messages_mutex_);
  auto it = std::find_if(messages_.begin(), messages_.end(), [&](const auto& existing_message) {
    return existing_message.local_id == message.local_id;  // id from server here can be null
  });
  LOG_INFO("To Save message text {}, id{}, local_id{}, and sended_status is {}",
           message.text.toStdString(),
           message.id,
           message.local_id.toStdString(),
           message.status_sended);
  if (it != messages_.end()) {
    LOG_INFO("Message {} already exist", message.text.toStdString());
    DBC_REQUIRE(it->id == 0 || it->id == message.id);
    it->updateFrom(message);
  } else {
    LOG_INFO("Message {} added", message.text.toStdString());
    messages_.push_back(message);
  }

  Q_EMIT messageAdded(message);  // todo: messageAdded :->: messageSaved() -> updateChatIconInList
}

void DataManager::deleteMessage(const Message& msg) {
  const std::lock_guard<std::mutex> lock(messages_mutex_);
  auto it = std::find_if(messages_.begin(), messages_.end(), [&](const Message& existing_message) {
    return existing_message.local_id == msg.local_id;  // id from server here can be null
  });

  if (it != messages_.end()) {
    LOG_INFO("Delete message {}", msg.toString());
    messages_.erase(it);
  } else {
    LOG_WARN("Message {} to delete not found", msg.toString());
    return;
  }

  Q_EMIT messageDeleted(msg);
}
void DataManager::readMessage(long long message_id, long long readed_by) {
  DBC_REQUIRE(message_id > 0);
  DBC_REQUIRE(readed_by > 0);
  const std::lock_guard<std::mutex> lock(messages_mutex_);
  auto it = std::find_if(messages_.begin(), messages_.end(), [&](const auto& existing_message) {
    return existing_message.id == message_id;
  });

  if (it == messages_.end()) {
    LOG_WARN("To read message with id {} not found",
             message_id);  // can be if u delete message for yourself
    return;
  }

  if (!it->readed_by_me) {
    it->read_counter++;
    it->readed_by_me = true;
    LOG_INFO("{} is marked readed", it->toString());
    Q_EMIT messageAdded(*it);  // todo: rename messageChanged

    // auto* model = getMessageModel(it->chat_id);
    // model->updateMessage(*it);
  } else {
    LOG_INFO("{} is already marked readed", it->toString());
  }
}
