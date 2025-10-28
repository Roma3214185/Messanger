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

  explicit MessageModel(QObject* parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
  [[nodiscard]] int rowCount() const {return rowCount(QModelIndex()); }
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  void addMessage(Message msg, const User& user, bool in_front = true);
  void clear();
  [[nodiscard]] std::optional<Message> getLastMessage();
  [[nodiscard]] std::optional<Message> getFirstMessage();
  static void setCurrentUserId(int user_id);
  void resetCurrentUseId();

 private:
  ListOfMessages messages_;
  UsersByMessageId users_by_message_id_;
  static std::optional<int> currentUserId;
};

#endif  // MESSAGEMODEL_H
