#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractItemModel>

#include "dto/Message.h"
#include "model.h"
#include "models/UserModel.h"

using ListOfMessages = QList<Message>;
using MessageId = long long;
using ChatId = long long;
using UsersByMessageId = std::unordered_map<MessageId, User>;
using MessagesByChatId = std::unordered_map<ChatId, ListOfMessages>;

class MessageModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    MessageIdRole = Qt::UserRole + 1,
    TextRole,
    TimestampRole,
    SenderIdRole,
    SendedStatusRole,
    ReadedStatusRole,
    FullMessage
  };

  explicit MessageModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QModelIndex indexFromId(MessageId) const;
  QHash<int, QByteArray> roleNames() const override;
  void saveMessage(const Message &msg);
  void deleteMessage(const Message &msg);
  void clear();
  std::optional<Message> getLastMessage() const;
  std::optional<Message> getOldestMessage() const;
  [[nodiscard]] ListOfMessages messages() const noexcept { return messages_; }

private:
  void sortMessagesByTimestamp();

  std::mutex messages_mutex_;
  ListOfMessages messages_;
};

#endif // MESSAGEMODEL_H
