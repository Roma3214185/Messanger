#ifndef CROWSOCKET_H
#define CROWSOCKET_H

#include <crow.h>
#include "interfaces/ISocket.h"

using WebsocketPtr = crow::websocket::connection*;

class CrowSocket : public ISocket {
    WebsocketPtr conn_;
  public:
    explicit CrowSocket(WebsocketPtr conn) : conn_(conn) {}

    void send_text(const std::string& text) override {
      conn_->send_text(text);
    }
};

#endif // CROWSOCKET_H
