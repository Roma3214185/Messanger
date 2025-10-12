#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <QString>

struct Message{
    int id;
    int chatId;
    int senderId;
    std::string text;
    int receiverId;
    QString timestamp;
};

#endif // MESSAGE_H
