#include "notificationservice/managers/socketmanager.h"

#include "Debug_profiling.h"
#include "notificationservice/CrowSocket.h"

void SocketsManager::saveConnections(UserId user_id, SocketPtr socket) {
 std::scoped_lock lock(ws_mutex_);
  user_sockets_[user_id] = socket;
}

void SocketsManager::deleteConnections(SocketPtr conn_to_delete) {  // todo: on close user send message (e.g "deinit")
 std::scoped_lock lock(ws_mutex_);

  auto it = std::ranges::find_if(user_sockets_,
                         [&](const auto &p) { return p.second == conn_to_delete; });
  if (it != user_sockets_.end()) {
    user_sockets_.erase(it);
    LOG_INFO("Deleted connection");
  } else {
    LOG_WARN("Connection to delete not found");
  }
}

bool SocketsManager::userOnline(UserId user_id) { return user_sockets_.contains(user_id); }

SocketPtr SocketsManager::getUserSocket(UserId user_id) {
 std::scoped_lock lock(ws_mutex_);
  auto find = user_sockets_.find(user_id);
  if (find == user_sockets_.end()) {
    return nullptr;
  }
  return find->second;
}
