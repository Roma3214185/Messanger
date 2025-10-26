#ifndef DRAWDATA_H
#define DRAWDATA_H

#include <QRect>
#include <QString>
#include <QDateTime>

struct MessageDrawData {
    QString username;
    QString text;
    QString avatar_path;
    QString timestamp;
    int sender_id;
    int receiver_id;
    bool is_mine;
};

struct UserDrawData{
    QString name;
    QPixmap avatar;
    QString tag;
};

struct ChatDrawData{
    QString title;
    QString last_message;
    QString avatar_path;
    QDateTime time;
    int unread;
};

#endif // DRAWDATA_H
