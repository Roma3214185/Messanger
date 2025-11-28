#pragma once

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include <unordered_map>

using ClientSocket = crow::websocket::connection;
using BackendSocket = std::shared_ptr<ix::WebSocket>;
using SocketConnectionsMap = std::unordered_map<ClientSocket*, BackendSocket>;
using Url = std::string;

class WebSocketBridge {
 public:
  explicit WebSocketBridge(const Url&);

  void onClientConnect(ClientSocket& client);
  void onClientMessage(ClientSocket& client, const std::string& data);
  void onClientClose(ClientSocket& client, const std::string& reason, uint16_t code);

 private:
  SocketConnectionsMap connections_;
  Url backend_url_;

  BackendSocket createBackendConnection(ClientSocket& client);
};
