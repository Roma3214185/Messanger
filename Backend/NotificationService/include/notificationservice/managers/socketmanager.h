#ifndef BACKEND_NOTIFICATION_SOCKETMANAGER_H
#define BACKEND_NOTIFICATION_SOCKETMANAGER_H

#include <unordered_map>
#include <memory>

#include "interfaces/ISocket.h"

using UserId = int;
using SocketPtr = std::shared_ptr<ISocket>;
using Sockets = std::unordered_map<int, SocketPtr>;

class SocketsManager {
 public:
  void      saveConnections(UserId, SocketPtr socket);
  void      deleteConnections(SocketPtr conn);
  SocketPtr  getUserSocket(UserId);
  bool userOnline(UserId);

 private:
  Sockets user_sockets_;
};

#endif  // BACKEND_NOTIFICATION_SOCKETMANAGER_H
