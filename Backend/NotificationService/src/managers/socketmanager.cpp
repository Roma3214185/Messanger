#include "notificationservice/managers/socketmanager.h"
#include "Debug_profiling.h"

void SocketsManager::saveConnections(UserId user_id, SocketPtr socket) {
  user_sockets_[user_id] = socket;
}

void SocketsManager::deleteConnections(SocketPtr conn_to_delete) {
  for (auto it = user_sockets_.begin(); it != user_sockets_.end(); ++it) {
    if (it->second == conn_to_delete) {
      user_sockets_.erase(it);
      return;
    }
  }
  LOG_WARN("Connection to delete not found");
}

bool SocketsManager::userOnline(UserId user_id) {
  return user_sockets_.find(user_id) != user_sockets_.end();
}

SocketPtr SocketsManager::getUserSocket(UserId user_id) {
  auto find = user_sockets_.find(user_id);
  if (find == user_sockets_.end()) {
    return nullptr;
  }
  return find->second;
}
