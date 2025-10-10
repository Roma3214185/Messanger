#ifndef CHATBASE_H
#define CHATBASE_H

#include <QString>
#include <QDateTime>

struct ChatBase {
    int chatId;
    QString title;
    QString lastMessage;
    int unread = 0;
    QDateTime lastMessageTime;
    QString avatarPath;
    virtual ~ChatBase() = default;
    virtual bool isPrivate() const = 0;
};

struct PrivateChat : public ChatBase {
    QString userTag;
    int userId;
    QString status;
    bool isPrivate() const override { return true; }
};

struct GroupChat : public ChatBase {
    int memberCount = 0;
    QVector<QString> memberTags;
    QVector<int> membersId;
    QVector<QString> avatarPaths;
    bool isPrivate() const override { return false; }
};

#endif // CHATBASE_H
