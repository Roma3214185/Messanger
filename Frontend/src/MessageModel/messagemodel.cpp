#include "messagemodel.h"

#include <QDateTime>
#include <QPixmap>

#include "Debug_profiling.h"

std::optional<int> MessageModel::currentUserId = std::nullopt;

MessageModel::MessageModel(QObject* parent) : QAbstractListModel(parent) {}

QVariant MessageModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= messages_.size()) return QVariant();

  const auto& msg = messages_[index.row()];

  switch (role) {
    case UsernameRole:
      return users_by_message_id_.at(msg.id).name;
    case TextRole:
      return msg.text;
    case AvatarRole:
      return users_by_message_id_.at(msg.id).avatarPath;
    case TimestampRole:
      return msg.timestamp;
    case SenderIdRole:
      return msg.senderId;
    case ReceiverIdTole:
      return *currentUserId;
    default:
      return QVariant();
  }
}

void MessageModel::setCurrentUserId(int user_id) { currentUserId = user_id; }

void MessageModel::resetCurrentUseId() { currentUserId = std::nullopt; }

std::optional<Message> MessageModel::getLastMessage() {
  if (messages_.empty()) return std::nullopt;
  return messages_.back();
}

std::optional<Message> MessageModel::getFirstMessage() {
  if (messages_.empty()) return std::nullopt;
  return messages_.front();
}

void MessageModel::addMessage(Message msg,
                              const User& user, bool in_front) {
  auto it = users_by_message_id_.find(msg.id);

  if (it != users_by_message_id_.end()) {
    // undelivered messages will not copy when i load all messages;
    LOG_WARN("Message '{}' with id '{}' already exist", msg.text.toStdString(),
             msg.id);
    return;
  }

  beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
  if (in_front) {
    messages_.push_front(msg);
  } else {
    messages_.push_back(msg);
  }
  users_by_message_id_[msg.id] = user;
  endInsertRows();
}

void MessageModel::clear() {
  if (messages_.isEmpty()) {
    return;
  }

  beginRemoveRows(QModelIndex(), 0, messages_.size() - 1);
  messages_.clear();
  users_by_message_id_.clear();
  endRemoveRows();
}

QHash<int, QByteArray> MessageModel::roleNames() const {
  return {{UsernameRole, "username"},      {TextRole, "text"},
          {AvatarRole, "avatar"},          {TimestampRole, "timestamp"},
          {ReceiverIdTole, "receiver_id"}, {SenderIdRole, "sender_id"}};
}

int MessageModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return messages_.size();
}
