#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QPixmap>

#include "headers/ChatBase.h"

using ChatPtr = std::shared_ptr<ChatBase>;
using ListOfChats = QList<ChatPtr>;
using std::optional;
using ChatIndex = size_t;
using OptionalChatIndex = std::optional<ChatIndex>;

class ChatModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum Roles {
    ChatIdRole = Qt::UserRole + 1,
    TitleRole,
    LastMessageRole,
    UnreadRole,
    LastMessageTimeRole,
    AvatarRole
  };

  explicit ChatModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  void addChat(const ChatPtr& chat);
  void updateChat(int chat_id, const QString& last_message,
                  const QDateTime& time);
  void addChatInFront(ChatPtr& chat);
  void realocateChatInFront(int chat_id);
  void clear();
  void sortChats();
  [[nodiscard]] OptionalChatIndex findIndexByChatId(int chat_id) const;

 Q_SIGNALS:
  void chatUpdated(int chat_id);

 private:
  ListOfChats chats_;
};

#endif  // CHATMODEL_H
