#include "notificationmanager.h"
#include "networkmanager.h"
#include "rabbitmqclient.h"
#include "SocketManager.h"

NotificationManager::NotificationManager(RabbitMQClient& mq, SocketsManager& sockManager, NetworkManager& networkManager)
    : mq(mq)
    , socketManager(sockManager)
    , networkManager(networkManager)
{

}

void NotificationManager::notifyMessageRead(int chatId, const MessageStatus& status){

}

void NotificationManager::notifyNewMessages(Message msg, int userId){

}

void NotificationManager::saveConnections(int userId, WebsocketPtr socket){
    socketManager.saveConnections(userId, socket);
}

void NotificationManager::deleteConnections(WebsocketPtr conn){
    socketManager.deleteConnections(conn);
}

void NotificationManager::userConnected(int userId, WebsocketPtr conn){
    saveConnections(userId, conn);
    // notify users who communicate with this user
}

void NotificationManager::onMarkReadMessage(Message message, int readBy){
    MessageStatus messageStatus{
        .id = message.id,
        .receiver_id = readBy,
        .is_read = true,
        .read_at = QDateTime::currentDateTime().toSecsSinceEpoch()
    };

    //manager.saveMessageStatus(status);
    notifyMessageRead(message.id, messageStatus);
}

void NotificationManager::onSendMessage(Message msg){
    PROFILE_SCOPE("Controller::onSendMessage");
    LOG_INFO("Send message from '{}' to chatId '{}' (text: '{}')", msg.sender_id, msg.chat_id, msg.text);

    //sendMessage(mq, msg);

    mq.subscribe("notification_service.in", [this](const std::string& body) {
        auto res = nlohmann::json::parse(body);
        if (res["event"] == "saved") {
            LOG_INFO("Saved accept from mq");
            if(res["saved"] == "User") {
                //User user = from_json();
                onUserSaved();
            }
            else if(res["saved"] == "Message") {
                Message newMsg;
                from_json(res, newMsg);
                onMessageSaved(newMsg);
            }
            else if(res["saved"] == "MessageStatus") {

                onMessageStatusSaved();
            }
            else LOG_ERROR("Onknow type of saved enity");
        }
    });
}

void NotificationManager::onMessageStatusSaved(){

}

void NotificationManager::onMessageSaved(Message msg){
    auto members_of_chat = networkManager.getMembersOfChat(msg.chat_id);

    LOG_INFO("Message('{}') is saved with id '{}'", msg.text, msg.id);
    LOG_INFO("For chat id '{}' finded '{}' members", msg.chat_id, members_of_chat.size());

    for(auto toUser: members_of_chat){
        LOG_INFO("Chat id: '{}'; member is ", msg.chat_id, toUser);

        MessageStatus msgStatus{
            .id = msg.id,
            .receiver_id = toUser,
            .is_read = false
        };

        saveMessageStatus(msgStatus);
        sendMessageToUser(toUser, msg);
    }
}

void NotificationManager::sendMessageToUser(int userId, Message& msg){
    auto userSocket = socketManager.getUserSocket(userId);
    if(!userSocket){
        LOG_INFO("User offline");
        return;
    }

    nlohmann::json j;
    to_json(j, msg);
    userSocket->send_text(j.dump());
}

void NotificationManager::saveMessage(Message& msg) {
    auto to_save = nlohmann::json{
        {"event", "save_message"}
    };
    to_json(to_save, msg);
    mq.publish("app.events", "save", to_save.dump());
}

void NotificationManager::saveMessageStatus(MessageStatus& msg) {
    auto to_save = nlohmann::json{
        {"event", "save_message_status"}
    };
    to_json(to_save, msg);
    mq.publish("app.events", "save", to_save.dump());
}

void NotificationManager::onUserSaved(){

}
