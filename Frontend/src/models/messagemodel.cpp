#include "models/messagemodel.h"

#include <QDateTime>
#include <QPixmap>

#include "Debug_profiling.h"

// std::optional<int> MessageModel::currentUserId = std::nullopt;

MessageModel::MessageModel(QObject *parent) : QAbstractListModel(parent) {}

QVariant MessageModel::data(const QModelIndex &index, int role) const {
  // DBC_REQUIRE(index.isValid() && index.row() < messages_.size());
  if (!index.isValid() || index.row() < 0 || index.row() >= messages_.size()) return QVariant();

  const Message msg = messages_.at(index.row());

  switch (role) {
    case MessageIdRole:
      return msg.id;
    case TextRole:
      return msg.text;
    case TimestampRole:
      return msg.timestamp;
    case SenderIdRole:
      return msg.sender_id;
    case SendedStatusRole:
      return msg.status_sended;
    case ReadedStatusRole:
      return false;  // TODO(roma) implement reading status(readed_cnt ?= 2
    case FullMessage:
      return QVariant::fromValue(msg);
    default:
      return QVariant();
  }
}

QModelIndex MessageModel::indexFromId(long long messageId) const {
  for (int row = 0; row < messages_.size(); ++row) {
    if (messages_[row].id == messageId) return index(row);
  }

  return QModelIndex();  // Not found
}

// void MessageModel::setCurrentUserId(long long user_id) { currentUserId =
// user_id; }

// void MessageModel::resetCurrentUseId() { currentUserId = std::nullopt; }

std::optional<Message> MessageModel::getLastMessage() const {
  if (messages_.empty()) return std::nullopt;
  return messages_.back();
}

std::optional<Message> MessageModel::getOldestMessage() const {
  if (messages_.empty()) return std::nullopt;
  return messages_.front();
}

void MessageModel::saveMessage(const Message &msg /*, const User& user*/) {
  LOG_INFO("[MessageModel::saveMessage]Msg id {} ans local_id {}, and status {}", msg.id, msg.local_id.toStdString(),
           msg.status_sended);

  const std::lock_guard<std::mutex> lock(messages_mutex_);
  auto it = std::find_if(messages_.begin(), messages_.end(), [&](const auto &other) {
    return msg.local_id == other.local_id;  //|| msg.id == other.id;
  });

  if (it != messages_.end()) {
    LOG_INFO("Message already exist with id {} ans local id {} and text {}", it->id, it->local_id.toStdString(),
             it->text.toStdString());
    beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
    it->updateFrom(msg);
    // it->receiver_read_status = msg.receiver_read_status;
    // it->read_counter = msg.read_counter;
    endInsertRows();
    return;
  }

  beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
  messages_.push_front(msg);
  sortMessagesByTimestamp();
  endInsertRows();
}

void MessageModel::deleteMessage(const Message &message) {
  LOG_INFO("Message model try to delete message {}", message.toString());

  const std::lock_guard<std::mutex> lock(messages_mutex_);
  auto it = std::find_if(messages_.begin(), messages_.end(),
                         [&](const auto &other) { return message.local_id == other.local_id; });

  if (it == messages_.end()) {
    LOG_INFO("Message already doesn't exist)");
    return;
  }

  const int row = std::distance(messages_.begin(), it);

  beginRemoveRows(QModelIndex(), row, row);
  messages_.erase(it);
  endRemoveRows();
}

void MessageModel::sortMessagesByTimestamp() {
  std::sort(messages_.begin(), messages_.end(), [](const auto &a, const auto &b) { return a.timestamp < b.timestamp; });

  // const std::lock_guard<std::mutex> lock(messages_mutex_);
  // std::sort(messages_.begin(), messages_.end(), [](const auto& first_message,
  // const auto& second_message){
  //   return first_message.timestamp < second_message.timestamp;
  // });
}

void MessageModel::clear() {
  if (messages_.isEmpty()) {
    return;
  }

  beginRemoveRows(QModelIndex(), 0, messages_.size() - 1);
  messages_.clear();
  // users_by_message_id_.clear();
  endRemoveRows();

  DBC_ENSURE(messages_.isEmpty());
}

QHash<int, QByteArray> MessageModel::roleNames() const {
  return {{TextRole, "text"}, {TimestampRole, "timestamp"}, {SenderIdRole, "sender_id"}};
}

int MessageModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return (int)messages_.size();
}
