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
    case SendedStatusRole:
      return msg.status_sended;
    case ReadedStatusRole:
      return false;  //TODO(roma) implement reading status(readed_cnt ?= 2
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
  users_by_message_id_[msg.id] = user;

  auto it = std::find_if(messages_.begin(), messages_.end(), [&] (const auto& other){
      return msg.local_id == other.local_id || msg.id == other.id;
  });

  LOG_INFO("Local_id {} and id {}", msg.local_id.toStdString(), msg.id);

  if (it != messages_.end()) {
    LOG_INFO("Message already exist with id {} ans local id {}", it->id, it->local_id.toStdString());
    LOG_INFO("Situation with id for text {} id {} vs other.id {}"
              "ans local_id {} vs other local_id {}", msg.id, it->id, msg.local_id.toStdString(),
              it->local_id.toStdString());

    if(msg.local_id != it->local_id || (msg.id != it->id && it->id != 0)) {
      LOG_ERROR("Invalid siruation with ids");
      //throw std::runtime_error("Invalid ids");
    }
    beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
    it->id = msg.id;
    it->text = msg.text;
    it->timestamp = msg.timestamp;
    it->status_sended = true;
    endInsertRows();
    return;
  }


  beginInsertRows(QModelIndex(), messages_.size(), messages_.size());
  if (in_front) {
    messages_.push_front(msg);
  } else {
    messages_.push_back(msg);
  }

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
