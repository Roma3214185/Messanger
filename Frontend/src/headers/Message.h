#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>

struct Message {
    int id;
    int senderId;
    int chatId;
    QString text;
    QDateTime timestamp;
};

#endif // MESSAGE_H
