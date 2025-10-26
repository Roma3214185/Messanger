#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractItemModel>

#include "Model/model.h"
#include "UserModel/UserModel.h"
#include "headers/Message.h"

using ListOfMessages = QList<Message>;
using MessageId = int;
using UsersByMessageId = std::unordered_map<MessageId, User>;

class MessageModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum Roles {
    UsernameRole = Qt::UserRole + 1,
    TextRole,
    AvatarRole,
    TimestampRole,
    ReceiverIdTole,
    SenderIdRole
  };

  explicit MessageModel(QObject* parent);
  MessageModel();

  [[nondiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
  [[nondiscard]] auto rowCount() const -> int {return rowCount(QModelIndex()); }
  [[nondiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
  [[nondiscard]] auto roleNames() const -> QHash<int, QByteArray> override;
  void addMessage(Message msg, const User& user);
  void addMessageInBack(Message msg, const User& user);
  void clear();
  [[nondiscard]] auto getLastMessage() -> std::optional<Message>;
  [[nondiscard]] auto getFirstMessage() -> std::optional<Message>;
  static void setCurrentUserId(int user_id);
  void resetCurrentUseId();

 private:
  ListOfMessages messages_;
  UsersByMessageId usersByMessageId;
  static std::optional<int> currentUserId;
};

#endif  // MESSAGEMODEL_H
