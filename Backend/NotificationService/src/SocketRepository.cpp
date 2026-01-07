#include "notificationservice/SocketRepository.h"

#include "notificationservice/CrowSocket.h"

SocketPtr SocketRepository::findSocket(crow::websocket::connection *conn) {
  std::lock_guard<std::mutex> lock(ws_mutex);
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
  std::lock_guard<std::mutex> lock(ws_mutex);
  active_sockets_.insert(socket);
}

void SocketRepository::deleteConnection(const SocketPtr &socket) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  active_sockets_.erase(socket);
}
