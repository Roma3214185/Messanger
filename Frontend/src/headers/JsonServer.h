#ifndef JSONSERVER_H
#define JSONSERVER_H
#include "headers/User.h"
#include "ChatModel/chatmodel.h"
#include <MessageModel/messagemodel.h>
#include <QString>
#include <QDateTime>

namespace JsonServer {

User getUserFromResponce(QJsonObject res){
    return User{
        .email = res["email"].toString(),
        .tag = res["tag"].toString(),
        .name = res["name"].toString(),
        .id = res["id"].toInt()
    };
}

Message getMessageFromJson(QJsonObject obj){
    Message newMsg{                                         // make factory
        .id = obj["message_id"].toInt(),
        .senderId = obj["sender_id"].toInt(),
        .chatId = obj["chat_id"].toInt(),
        .text = obj["text"].toString(),
        .timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate)
    };
    qDebug() << "[INFO2] timestamp = " << newMsg.timestamp;
    return newMsg;
}

std::shared_ptr<ChatBase> getChatFromJson(QJsonObject obj){
    auto type = obj["type"].toString();

    if(type == "private"){                                                  // make factory
        auto newChat = std::make_shared<PrivateChat>();
        newChat->title = obj["user"].toObject()["name"].toString();
        newChat->avatarPath = obj["user"].toObject()["avatar"].toString();
        newChat->chatId = obj["id"].toInt();
        newChat->userId = obj["user"].toObject()["id"].toInt();
        qDebug() << "[INFO] Load private chat with name: " << newChat->title << " and id " << newChat->chatId;
        return newChat;
    }else{
        auto newChat = std::make_shared<GroupChat>();
        newChat->chatId = obj["id"].toInt();
        newChat->title = obj["name"].toString();
        newChat->avatarPath = obj["avatar"].toString();
        newChat->memberCount = obj["member_count"].toInt();
        qDebug() << "[INFO] Load group chat with name: " << newChat->title << " and id " << newChat->chatId;
        return newChat;
    }

    return nullptr;
}

std::shared_ptr<ChatBase> getPrivateChatFromJson(QJsonObject obj){
    auto newChat = std::make_shared<PrivateChat>();
    newChat->userId = obj["user_id"].toInt();
    newChat->chatId = obj["chat_id"].toInt();
    newChat->title = obj["title"].toString();
    newChat->avatarPath = obj["avatar"].toString();
    return newChat;
}

} //namespace JsonServer

#endif // JSONSERVER_H
