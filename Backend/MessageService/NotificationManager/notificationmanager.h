#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H
#ifdef signals
#undef signals
#endif
#ifdef slots
#undef slots
#endif
#ifdef emit
#undef emit
#endif
#include <crow.h>

#include <unordered_map>
#include "../Headers/Message.h"
#include <NetworkManager.h>
#include "../MessageManager/MessageManager.h"

using WebsocketPtr = crow::websocket::connection*;
using UserId = int;
using WebsocketByIdMap = std::unordered_map<UserId, WebsocketPtr>;

class NotificationManager {
public:
    NotificationManager(MessageManager& manager) : manager(manager){}

    void notifyMessageRead(int chatId, const MessageStatus& status);
    void notifyNewMessages(Message msg, int userId);

    void saveConnections(int userId, WebsocketPtr socket){
        userSockets[userId] = socket;
    }

    void deleteConnections(WebsocketPtr conn){
        for (auto it = userSockets.begin(); it != userSockets.end(); ++it) {
            if (it->second == conn) {
                userSockets.erase(it);
                break;
            }
        }
    }

private:
    WebsocketByIdMap userSockets;
    MessageManager& manager;
};

#endif // NOTIFICATIONMANAGER_H
