#ifndef CHATBASE_H
#define CHATBASE_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

struct ChatBase {
  int       chat_id;
  QString   title;
  QString   last_message;
  int       unread = 0;
  QDateTime last_message_time;
  QString   avatar_path;

  virtual ~ChatBase()                                  = 0;
  [[nodiscard]] virtual auto isPrivate() const -> bool = 0;
};

struct PrivateChat : public ChatBase {
  QString user_tag;
  int     user_id;
  QString status;

  [[nodiscard]] auto isPrivate() const -> bool override { return true; }
};

struct GroupChat : public ChatBase {
  int          member_count = 0;
  QStringList  member_tags;
  QVector<int> members_id;
  QStringList  avatar_paths;

  [[nodiscard]] auto isPrivate() const -> bool override { return false; }
};

inline ChatBase::~ChatBase() = default;

#endif  // CHATBASE_H
