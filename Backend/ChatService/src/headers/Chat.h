#ifndef CHAT_H
#define CHAT_H

#include <string>
#include <QDateTime>

struct Chat{
    int id;
    bool isGroup;
    std::string name;
    std::string avatar;
    QDateTime createdAt;
};

#endif // CHAT_H
