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

  [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
  [[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
  [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;
  void addChat(const ChatPtr& chat);
  void updateChat(int chatId, const QString& lastMessage,
                  const QDateTime& time /*, int unread = 0,*/);
  void addChatInFront(ChatPtr& chat);
  void realocateChatInFront(int chatId);
  void clear();
  void sortChats();
  [[nodiscard]] auto findIndexByChatId(int chatId) const -> OptionalChatIndex;

 Q_SIGNALS:
  void chatUpdated(int chatId);

 private:
  ListOfChats m_chats;
};

#endif  // CHATMODEL_H
