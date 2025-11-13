#include "managers/datamanager.h"

#include "Debug_profiling.h"

ChatPtr DataManager::getPrivateChatWithUser(int user_id) {
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

int DataManager::getNumberOfExistingModels() const {
  return message_models_by_chat_id_.size();
}

MessageModelPtr DataManager::getMessageModel(int chat_id) {
  auto iter = message_models_by_chat_id_.find(chat_id);
  if (iter == message_models_by_chat_id_.end()) return nullptr;
  return iter->second;
}

ChatPtr DataManager::getChat(int chat_id) {
  auto chat_iter = chats_by_id_.find(chat_id);
  if (chat_iter == chats_by_id_.end()) return nullptr;
  return chat_iter->second;
}

int DataManager::getNumberOfExistingChats() const { return chats_by_id_.size(); }

int DataManager::getNumberOfExistingUsers() const { return users_by_id_.size(); }

void DataManager::clearAllChats() { chats_by_id_.clear(); }

void DataManager::clearAllMessageModels() { message_models_by_chat_id_.clear(); }

void DataManager::clearAllUsers() { users_by_id_.clear(); }

void DataManager::clearAll() {
  clearAllUsers();
  clearAllMessageModels();
  clearAllChats();
}

void DataManager::addChat(ChatPtr chat, MessageModelPtr message_model) {
  if(chat->chat_id <= 0) throw std::runtime_error("Invalid id to add chat");
  if (!message_model)
    message_model = std::make_shared<MessageModel>();

  chats_by_id_[chat->chat_id] = chat;
  message_models_by_chat_id_[chat->chat_id] = message_model;
}

void DataManager::saveUser(User user) {
  if(user.id <= 0) throw std::runtime_error("User id is invalid");
  users_by_id_[user.id] = user;
}

std::optional<User> DataManager::getUser(UserId user_id) {
  auto it = users_by_id_.find(user_id);
  return it == users_by_id_.end() ? std::nullopt : std::make_optional(it->second);
}

