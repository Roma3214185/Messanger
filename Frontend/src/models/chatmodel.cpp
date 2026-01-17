#include "models/chatmodel.h"

#include "Debug_profiling.h"

ChatModel::ChatModel(QObject *parent) : QAbstractListModel(parent) {}

int ChatModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return (int)chats_.size();
}

QVariant ChatModel::data(const QModelIndex &index, int role) const {
  // DBC_REQUIRE(index.isValid() && index.row() < chats_.size());
  if (!index.isValid() || index.row() >= chats_.size() || index.row() < 0) return QVariant();

  const auto &chat = chats_.at(index.row());

  switch (role) {
    case ChatIdRole:
      return chat->chat_id;
    case TitleRole:
      return chat->title;
    case LastMessageRole:
      if (chat->last_message.has_value())
        return QVariant::fromValue(chat->last_message.value());

      return QVariant();
    case UnreadRole:
      return chat->unread;
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
          {AvatarRole, "avatar"}};
}

void ChatModel::addChat(const ChatPtr &chat) {
  if (auto index = findIndexByChatId(chat->chat_id); index.has_value()) {
    LOG_WARN("Chat with id {} already exist");
    return;
  }

  beginInsertRows(QModelIndex(), chats_.size(), chats_.size());
  chats_.push_back(chat);
  sortChats();
  endInsertRows();
}

void ChatModel::sortChats() {
  beginInsertRows(QModelIndex(), chats_.size(), chats_.size());
  std::sort(chats_.begin(), chats_.end(), [&](const auto &chat1, const auto &chat2) {
    // if(Private) return for created time
    //  else retunr for joined time
    return chat1->last_message_time > chat2->last_message_time;
  });
  endInsertRows();
}

void ChatModel::updateChatInfo(const long long chat_id, const std::optional<Message> &message
                               /*, TODO: int unread = 0,*/) {
  if (message == std::nullopt) return;
  DBC_REQUIRE(chat_id > 0);

  auto it = std::find_if(chats_.begin(), chats_.end(), [&](const auto &chat) { return chat->chat_id == chat_id; });
  if (it == chats_.end()) return;
  // todo: make copy-asigned constructor
  (*it)->last_message = message;
  (*it)->unread = 0;  // unread++;

  sortChats();  // if delete was??? // what about focus???
  // QModelIndex idx = index(i);
  // Q_EMIT dataChanged(idx, idx);
  Q_EMIT chatUpdated(chat_id);
}

OptionalChatIndex ChatModel::findIndexByChatId(long long chat_id) const {
  DBC_REQUIRE(chat_id > 0);
  for (size_t i = 0; i < chats_.size(); ++i) {
    if (chats_[i]->chat_id == chat_id) {
      return i;
    }
  }

  return std::nullopt;
}

void ChatModel::clear() {
  if (!chats_.isEmpty()) {
    beginRemoveRows(QModelIndex(), 0, chats_.size() - 1);
    chats_.clear();
    endRemoveRows();
  }

  DBC_ENSURE(chats_.isEmpty());
}
