#ifndef CROWSOCKET_H
#define CROWSOCKET_H

#include <crow.h>

#include "Debug_profiling.h"
#include "interfaces/ISocket.h"

class CrowSocket final : public ISocket {
  crow::websocket::connection *conn_;
  std::mutex conn_mutex_;

 public:
  explicit CrowSocket(crow::websocket::connection *conn) : conn_(conn) {}

  bool isSameAs(crow::websocket::connection *other) { return other == conn_; }

  void send_text(const std::string &text) override {
    if (text.empty()) {
      LOG_WARN("Text to send is empty");
      return;
    }

    if (conn_) {
      conn_->send_text(text);
      LOG_INFO("{} is sended", text);
    } else {
      LOG_ERROR("{} is not sended, nullptr conn_", text);
    }
  }
};

#endif  // CROWSOCKET_H
