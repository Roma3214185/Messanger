#ifndef SOCKETREPOSITORY_H
#define SOCKETREPOSITORY_H

#include <crow.h>
#include <unordered_set>

#include "interfaces/ISocket.h"

using SocketPtr = std::shared_ptr<ISocket>;

class SocketRepository {
 public:
  SocketPtr findSocket(crow::websocket::connection *conn);
  void addConnection(const SocketPtr &socket);
  void deleteConnection(const SocketPtr &socket);

 private:
  std::unordered_set<SocketPtr> active_sockets_;
  std::mutex ws_mutex_;
};

#endif  // SOCKETREPOSITORY_H
