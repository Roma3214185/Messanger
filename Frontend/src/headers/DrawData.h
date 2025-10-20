#ifndef DRAWDATA_H
#define DRAWDATA_H

#include <QRect>
#include <QString>
#include <QDateTime>

struct MessageDrawData {
    QString username;
    QString text;
    QString avatarPath;
    QString timestamp;
    int senderId;
    int receiverId;
    bool isMine;
};

struct UserDrawData{
    QString name;
    QPixmap avatar;
    QString tag;
};

struct ChatDrawData{
    QString title;
    QString lastMessage;
    QString avatarPath;
    QDateTime time;
    int unread;
};

#endif // DRAWDATA_H
