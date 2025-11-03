#include "datamanager.h"

#include "DebugProfiling/Debug_profiling.h"

ChatPtr DataManager::getPrivateChatWithUser(int user_id){
  for (auto [_, chat] : chats_by_id_) {
    if (chat->isPrivate()) {
      auto* pchat = static_cast<PrivateChat*>(chat.get());
      if (pchat->user_id == user_id) {
        LOG_INFO("Found private chat for this user '{}' and id '{}'",
                 pchat->title.toStdString(), pchat->chat_id);
        return chat;
      }
    }
  }
  return nullptr;
}

MessageModelPtr DataManager::getMessageModel(int chat_id) {
  auto iter = message_models_by_chat_id_.find(chat_id);
  if(iter == message_models_by_chat_id_.end()) return nullptr;
  return iter->second;
}

void DataManager::addMessageModel(int chat_id, MessageModelPtr message_model) {
  message_models_by_chat_id_[chat_id] = message_model;
}

ChatPtr DataManager::getChat(int chat_id) {
  auto chat_iter = chats_by_id_.find(chat_id);
  if (chat_iter == chats_by_id_.end()) return nullptr;
  return chat_iter->second;
}

int DataManager::getNumberOfExistingChats() const {
  return chats_by_id_.size();
}

void DataManager::clearAllChats(){
  chats_by_id_.clear();
}

void DataManager::clearAllMessageModels() {
  message_models_by_chat_id_.clear();
}

void DataManager::addChat(ChatPtr chat) {
  chats_by_id_[chat->chat_id] = chat;
}
