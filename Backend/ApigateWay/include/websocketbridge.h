#pragma once

#include <unordered_map>

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include "Debug_profiling.h"

class WebSocketBridge {
  public:
    explicit WebSocketBridge(const std::string& backend_url)
        : backend_url_(backend_url) {}

    void onClientConnect(crow::websocket::connection& client);
    void onClientMessage(crow::websocket::connection& client, const std::string& data);
    void onClientClose(crow::websocket::connection& client, const std::string& reason, uint16_t code);

  private:
    std::unordered_map<crow::websocket::connection*, std::shared_ptr<ix::WebSocket>> connections_;
    std::string backend_url_;

    std::shared_ptr<ix::WebSocket> createBackendConnection(crow::websocket::connection& client);
};
