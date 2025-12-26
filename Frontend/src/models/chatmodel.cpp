#include "models/chatmodel.h"
#include "Debug_profiling.h"

ChatModel::ChatModel(QObject* parent) : QAbstractListModel(parent) {}

int ChatModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return (int)chats_.size();
}

QVariant ChatModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= chats_.size()) return QVariant();

  const auto& chat = chats_[index.row()];

  switch (role) {
    case ChatIdRole:
      return chat->chat_id;
    case TitleRole:
      return chat->title;
    case LastMessageRole:
      return chat->last_message;
    case UnreadRole:
      return chat->unread;
    case LastMessageTimeRole:
      return chat->last_message_time;
    case AvatarRole:
      return chat->avatar_path;
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> ChatModel::roleNames() const {
  return {{ChatIdRole, "chat_id"},
          {TitleRole, "title"},
          {LastMessageRole, "lastMessage"},
          {UnreadRole, "unread"},
          {LastMessageTimeRole, "lastMessageTime"},
          {AvatarRole, "avatar"}};
}

void ChatModel::addChat(const ChatPtr& chat) {
  for(auto existing_chat: chats_) {
    if(existing_chat->chat_id == chat->chat_id) {
      return LOG_ERROR("Chat with id {} already exist");
    }
  }

  beginInsertRows(QModelIndex(), chats_.size(), chats_.size());
  chats_.push_back(chat);
  sortChats();
  endInsertRows();
}

void ChatModel::sortChats() {
  beginInsertRows(QModelIndex(), chats_.size(), chats_.size());
  std::sort(chats_.begin(), chats_.end(), [&](const auto& chat1, const auto& chat2) {
    // if(Private) return for created time
    //  else retunr for joined time
    return chat1->last_message_time > chat2->last_message_time;
  });
  endInsertRows();
}

void ChatModel::updateChatInfo(const long long        chat_id,
                           const std::optional<Message>&  message
                           /*, TODO: int unread = 0,*/) {
  if(message == std::nullopt) return;
  int i = 0;
  for (; i < chats_.size(); i++) {
    if (chats_[i]->chat_id == chat_id) {
      chats_[i]->last_message      = message->text;
      chats_[i]->unread            = 0;  // unread++;
      chats_[i]->last_message_time = message->timestamp;
      break;
    }
  }
  if (i == chats_.size()) return;
  sortChats();  // if delete was??? // what about focus???
  QModelIndex idx = index(i);
  Q_EMIT dataChanged(idx, idx);
  Q_EMIT chatUpdated(chat_id);
}

OptionalChatIndex ChatModel::findIndexByChatId(long long chat_id) const {
  for (size_t i = 0; i < chats_.size(); ++i) {
    if (chats_[i]->chat_id == chat_id) {
      return i;
    }
  }

  return std::nullopt;
}

void ChatModel::clear() {
  if (chats_.isEmpty()) return;

  beginRemoveRows(QModelIndex(), 0, chats_.size() - 1);
  chats_.clear();
  endRemoveRows();
}
