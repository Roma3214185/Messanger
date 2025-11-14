#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <crow.h>

class ISocket {
  public:
    virtual void send_text(const std::string& text) = 0;
    virtual ~ISocket() = default;
};

using WebsocketPtr     = crow::websocket::connection*;

class CrowSocket : public ISocket {
    WebsocketPtr conn_;
  public:
    CrowSocket(WebsocketPtr conn) : conn_(conn) {}

    void send_text(const std::string& text) override {
      conn_->send_text(text);
    }
};

using UserId           = int;
using SocketsByIdMap = std::unordered_map<UserId, ISocket*>;

class SocketsManager {
 public:
  void      saveConnections(UserId, ISocket* socket);
  void      deleteConnections(ISocket* conn);
  ISocket*  getUserSocket(UserId);

 private:
  SocketsByIdMap user_sockets_;
};

#endif  // SOCKETMANAGER_H
