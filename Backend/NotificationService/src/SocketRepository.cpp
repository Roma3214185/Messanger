#include "notificationservice/SocketRepository.h"

#include "notificationservice/CrowSocket.h"
#include "Debug_profiling.h"

SocketPtr SocketRepository::findSocket(crow::websocket::connection *conn) {
  std::scoped_lock lock(ws_active_mutex_);
  for (auto &socket : active_sockets_) {
    if (auto crowSocket = std::dynamic_pointer_cast<CrowSocket>(socket)) {
      if (crowSocket->isSameAs(conn)) {
        return socket;
      }
    }
  }

  return nullptr;
}

void SocketRepository::addConnection(const SocketPtr &socket) {
  std::scoped_lock lock(ws_active_mutex_);
  active_sockets_.insert(socket);
}

void SocketRepository::saveConnections(UserId user_id, SocketPtr socket) {
    std::scoped_lock lock(ws_user_mutex_);
    user_sockets_[user_id] = std::move(socket);
}

void SocketRepository::deleteConnection(const SocketPtr &conn_to_delete) {  // todo: on close user send message (e.g "deinit")
    std::scoped_lock lock(ws_user_mutex_);
    std::scoped_lock lock2(ws_active_mutex_);

    active_sockets_.erase(conn_to_delete);

    auto it = std::ranges::find_if(user_sockets_, [&](const auto &p) { return p.second == conn_to_delete; });
    if (it != user_sockets_.end()) {
        user_sockets_.erase(it);
        LOG_INFO("Deleted connection");
    } else {
        LOG_WARN("Connection to delete not found");
    }
}

bool SocketRepository::userOnline(UserId user_id) {
    std::scoped_lock lock(ws_user_mutex_);
    return user_sockets_.contains(user_id);
}

SocketPtr SocketRepository::getUserSocket(UserId user_id) {
    std::scoped_lock lock(ws_user_mutex_);
    auto find = user_sockets_.find(user_id);
    if (find == user_sockets_.end()) {
        return nullptr;
    }
    return find->second;
}

