#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractItemModel>

#include "dto/Message.h"
#include "model.h"
#include "models/UserModel.h"

using ListOfMessages   = QList<Message>;
using MessageId        = long long;
using ChatId           = long long;
using UsersByMessageId = std::unordered_map<MessageId, User>;
using MessagesByChatId = std::unordered_map<ChatId, ListOfMessages>;

class MessageModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum Roles {
    UsernameRole = Qt::UserRole + 1,
    MessageIdRole,
    TextRole,
    AvatarRole,
    TimestampRole,
    ReceiverIdTole,
    SenderIdRole,
    SendedStatusRole,
    ReadedStatusRole
  };

  explicit MessageModel(QObject* parent = nullptr);

  int                    rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant               data(const QModelIndex& index, int role) const override;
  QModelIndex indexFromId(MessageId) const;
  QHash<int, QByteArray> roleNames() const override;
  void addMessage(const Message& msg, const User& user);
  void clear();
  std::optional<Message> getLastMessage() const;
  std::optional<Message> getOldestMessage() const;
  static void setCurrentUserId(long long user_id);
  void                                 resetCurrentUseId();

 private:
  void  sortMessagesByTimestamp();
  ListOfMessages            messages_;
  MessagesByChatId          messages_by_chat_id;
  UsersByMessageId          users_by_message_id_;
  static std::optional<int> currentUserId;
};

#endif  // MESSAGEMODEL_H
