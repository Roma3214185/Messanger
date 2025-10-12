#ifndef CHATBASE_H
#define CHATBASE_H

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QVector>

class ChatBase{

public:

    int chatId = 0;
    QString title;
    QString lastMessage;
    int unread = 0;
    QDateTime lastMessageTime;
    QString avatarPath;

    virtual ~ChatBase() = default;
    virtual bool isPrivate() const = 0;
};

class PrivateChat : public ChatBase{

public:

    QString userTag;
    int userId = 0;
    QString status;

    bool isPrivate() const override { return true; }
};

class GroupChat : public ChatBase{

public:

    int memberCount = 0;
    QStringList memberTags;
    QVector<int> membersId;
    QStringList avatarPaths;

    bool isPrivate() const override { return false; }
};

#endif // CHATBASE_H
