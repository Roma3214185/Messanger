#include "notificationservice/managers/socketmanager.h"

#include "Debug_profiling.h"
#include "notificationservice/CrowSocket.h"

void SocketsManager::saveConnections(UserId user_id, SocketPtr socket) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  user_sockets_[user_id] = socket;
}

void SocketsManager::deleteConnections(
    SocketPtr conn_to_delete) {  // todo: on close user send message (e.g "deinit")
  std::lock_guard<std::mutex> lock(ws_mutex);

  auto it = std::find_if(user_sockets_.begin(), user_sockets_.end(), [&](const auto& p) {
    return p.second == conn_to_delete;
  });
  if (it != user_sockets_.end()) {
    user_sockets_.erase(it);
    LOG_INFO("Deleted connection");
  } else {
    LOG_WARN("Connection to delete not found");
  }
}

bool SocketsManager::userOnline(UserId user_id) {
  return user_sockets_.find(user_id) != user_sockets_.end();
}

SocketPtr SocketsManager::getUserSocket(UserId user_id) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  auto                        find = user_sockets_.find(user_id);
  if (find == user_sockets_.end()) {
    return nullptr;
  }
  return find->second;
}
