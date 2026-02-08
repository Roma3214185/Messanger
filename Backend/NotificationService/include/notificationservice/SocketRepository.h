#ifndef SOCKETREPOSITORY_H
#define SOCKETREPOSITORY_H

#include <crow.h>
#include <unordered_set>

#include "interfaces/ISocket.h"

using SocketPtr = std::shared_ptr<ISocket>;
using UserId = long long;
using UserSocketsMap = std::unordered_map<UserId, SocketPtr>;
using AvtiveSockets = std::unordered_set<SocketPtr>;

class IActiveSocketRepository {
 public:
  virtual ~IActiveSocketRepository() = default;
  virtual SocketPtr findSocket(crow::websocket::connection *conn) = 0;
  virtual void addConnection(const SocketPtr &socket) = 0;
  virtual void deleteConnection(const SocketPtr &socket) = 0;
};

class IUserSocketRepository {
 public:
  virtual ~IUserSocketRepository() = default;
  virtual void saveConnections(UserId, SocketPtr socket) = 0;
  virtual SocketPtr getUserSocket(UserId) = 0;
  virtual bool userOnline(UserId) = 0;
};

class SocketRepository : public IActiveSocketRepository, public IUserSocketRepository {
 public:
  SocketPtr findSocket(crow::websocket::connection *conn) override;
  void addConnection(const SocketPtr &socket) override;
  void deleteConnection(const SocketPtr &socket) override;
  void saveConnections(UserId, SocketPtr socket) override;
  SocketPtr getUserSocket(UserId) override;
  bool userOnline(UserId) override;

 private:
  void deleteFromActiveConnections(const SocketPtr &conn_to_delete);
  void deleteFromUserSocketsMap(const SocketPtr &conn_to_delete);

 private:
  AvtiveSockets active_sockets_;
  UserSocketsMap user_sockets_;
  std::mutex ws_active_mutex_;
  std::mutex ws_user_mutex_;
};

#endif  // SOCKETREPOSITORY_H
