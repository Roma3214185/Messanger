#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <unordered_map>

#include "interfaces/ISocket.h"

using UserId = int;
using SocketsByIdMap = std::unordered_map<UserId, ISocket*>;

class SocketsManager {
 public:
  void      saveConnections(UserId, ISocket* socket);
  void      deleteConnections(ISocket* conn);
  ISocket*  getUserSocket(UserId);
  bool userOnline(UserId);

 private:
  SocketsByIdMap user_sockets_;
};

#endif  // SOCKETMANAGER_H
