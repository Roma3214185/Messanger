#ifndef CROWSOCKET_H
#define CROWSOCKET_H

#include <crow.h>
#include "interfaces/ISocket.h"
#include "Debug_profiling.h"

using WebsocketPtr = crow::websocket::connection*;

class CrowSocket : public ISocket {
    WebsocketPtr conn_;
  public:
    explicit CrowSocket(WebsocketPtr conn) : conn_(conn) {}

    void send_text(const std::string& text) override {
      LOG_INFO("In socket send text: {}", text);
      if (!conn_) {
        LOG_WARN("Attempt send to CLOSED socket: {}", text);
        return;
      }

      try {
        conn_->send_text(text);
      } catch (...) {
        LOG_ERROR("Crow crashed sending text");
      }

      LOG_INFO("Text is sended");
    }
};

#endif // CROWSOCKET_H
