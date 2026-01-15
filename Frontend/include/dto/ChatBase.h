#ifndef CHATBASE_H
#define CHATBASE_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>
#include "entities/ReactionInfo.h"

struct ChatBase {
  long long chat_id{0};  // todo: make member only id
  QString title;
  QString last_message;
  int unread{0};
  QDateTime last_message_time;
  QString avatar_path = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
  std::vector<ReactionInfo> default_reactions;

  virtual ~ChatBase() = 0;
  [[nodiscard]] virtual auto isPrivate() const -> bool = 0;

  virtual bool checkInvariants() { return chat_id > 0 && !title.isEmpty() && !avatar_path.isEmpty(); }
};

struct PrivateChat : public ChatBase {
  QString user_tag;
  long long user_id{0};
  QString status;

  [[nodiscard]] auto isPrivate() const -> bool override { return true; }
};

struct GroupChat : public ChatBase {
  int member_count{0};
  QStringList member_tags;
  QVector<int> members_id;
  QStringList avatar_paths;
  // in future admins override std::vector<ReactionInfo> default_reactions;

  [[nodiscard]] auto isPrivate() const -> bool override { return false; }
};

using ChatPtr = std::shared_ptr<ChatBase>;

inline ChatBase::~ChatBase() = default;

class ChatFactory {
 public:
  static ChatPtr createPrivateChat(const long long chat_id, const QString &title, const QString &user_tag,
                                   const long long user_id, const QString &status, const QString &last_message = {},
                                   const QDateTime &last_message_time = QDateTime(), const QString &avatar_path = {}) {
    auto chat = std::make_shared<PrivateChat>();
    chat->chat_id = chat_id;
    chat->title = title;
    chat->user_tag = user_tag;
    chat->user_id = user_id;
    chat->status = status;
    chat->last_message = last_message;
    chat->last_message_time = last_message_time;
    chat->avatar_path = avatar_path;
    return chat;
  }

  static ChatPtr createGroupChat(const long long chat_id, const QString &title, const int member_count,
                                 const QStringList &member_tags, const QVector<int> &members_id,
                                 const QStringList &avatar_paths, const QString &last_message = {},
                                 const QDateTime &last_message_time = QDateTime(), const QString &avatar_path = {}) {
    auto chat = std::make_shared<GroupChat>();
    chat->chat_id = chat_id;
    chat->title = title;
    chat->member_count = member_count;
    chat->member_tags = member_tags;
    chat->members_id = members_id;
    chat->avatar_paths = avatar_paths;
    chat->last_message = last_message;
    chat->last_message_time = last_message_time;
    chat->avatar_path = avatar_path;
    return chat;
  }
};

#endif  // CHATBASE_H
