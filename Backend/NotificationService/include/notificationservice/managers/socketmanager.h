#ifndef BACKEND_NOTIFICATION_SOCKETMANAGER_H
#define BACKEND_NOTIFICATION_SOCKETMANAGER_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "interfaces/ISocket.h"

using UserId    = long long;
using SocketPtr = std::shared_ptr<ISocket>;
using Sockets   = std::unordered_map<UserId, SocketPtr>;

class SocketsManager {
 public:
  void      saveConnections(UserId, SocketPtr socket);
  void      deleteConnections(SocketPtr conn);
  SocketPtr getUserSocket(UserId);
  bool      userOnline(UserId);

 private:
  Sockets    user_sockets_;
  std::mutex ws_mutex;
};

#endif  // BACKEND_NOTIFICATION_SOCKETMANAGER_H
