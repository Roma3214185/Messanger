#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <crow.h>

using WebsocketPtr     = crow::websocket::connection*;
using UserId           = int;
using WebsocketByIdMap = std::unordered_map<UserId, WebsocketPtr>;

class SocketsManager {
 public:
  void         saveConnections(int user_id, WebsocketPtr socket);
  void         deleteConnections(WebsocketPtr conn);
  WebsocketPtr getUserSocket(int user_id);

 private:
  WebsocketByIdMap user_sockets_;
};

#endif  // SOCKETMANAGER_H
