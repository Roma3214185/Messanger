#ifndef CROWSOCKET_H
#define CROWSOCKET_H

#include <crow.h>

#include "interfaces/ISocket.h"

class CrowSocket final : public ISocket {
 public:
  explicit CrowSocket(crow::websocket::connection *conn);

  bool isSameAs(crow::websocket::connection *other);
  void send_text(const std::string &text) override;

 private:
  crow::websocket::connection *conn_;
  std::mutex conn_mutex_;
};

#endif  // CROWSOCKET_H
