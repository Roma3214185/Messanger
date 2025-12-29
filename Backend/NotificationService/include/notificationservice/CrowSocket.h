#ifndef CROWSOCKET_H
#define CROWSOCKET_H

#include <crow.h>
#include "interfaces/ISocket.h"
#include "Debug_profiling.h"


class CrowSocket final : public ISocket {
     crow::websocket::connection& conn_;
  public:
    explicit CrowSocket(crow::websocket::connection& conn) : conn_(conn) {}

    void send_text(const std::string& text) override {
      if(text.empty()) {
        LOG_WARN("Text to send is empty");
        return;
      }

      try {
        conn_.send_text(text);
      } catch (...) {
        LOG_ERROR("Crow crashed sending text");
      }

      LOG_INFO("Text is sended");
    }
};

#endif // CROWSOCKET_H
