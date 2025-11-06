#include "managers/socketmanager.h"

void SocketsManager::saveConnections(int user_id, WebsocketPtr socket) {
  user_sockets_[user_id] = socket;
}

void SocketsManager::deleteConnections(WebsocketPtr conn) {
  for (auto it = user_sockets_.begin(); it != user_sockets_.end(); ++it) {
    if (it->second == conn) {
      user_sockets_.erase(it);
      break;
    }
  }
}

WebsocketPtr SocketsManager::getUserSocket(int user_id) {
  auto find = user_sockets_.find(user_id);
  if (find == user_sockets_.end()) return nullptr;
  return find->second;
}
