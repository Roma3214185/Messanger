#ifndef CHATBASE_H
#define CHATBASE_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

struct ChatBase {
  int chatId;
  QString title;
  QString lastMessage;
  int unread = 0;
  QDateTime lastMessageTime;
  QString avatarPath;

  virtual ~ChatBase() = 0;
  [[nondiscard]] virtual auto isPrivate() const -> bool = 0;
};

struct PrivateChat : public ChatBase {
  QString userTag;
  int userId;
  QString status;

  [[nondiscard]] auto isPrivate() const -> bool override { return true; }
};

struct GroupChat : public ChatBase {
  int memberCount = 0;
  QStringList memberTags;
  QVector<int> membersId;
  QStringList avatarPaths;

  [[nondiscard]] auto isPrivate() const -> bool override { return false; }
};

inline ChatBase::~ChatBase() = default;

#endif  // CHATBASE_H
