#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <crow/crow.h>

using WebsocketPtr = crow::websocket::connection*;
using UserId = int;
using WebsocketByIdMap = std::unordered_map<UserId, WebsocketPtr>;

class SocketsManager{
    WebsocketByIdMap userSockets;
public:
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

    WebsocketPtr getUserSocket(int userId){
        auto find = userSockets.find(userId);
        if (find == userSockets.end()) return nullptr;
        return find->second;
    }


};

#endif // SOCKETMANAGER_H
