#pragma once

#include <crow.h>
#include <ixwebsocket/IXWebSocket.h>

#include <unordered_map>

using ClientSocket = crow::websocket::connection;
using BackendSocket = std::shared_ptr<ix::WebSocket>;
using ClientId = std::string;
using BackendSocketConnectionsMap = std::unordered_map<ClientId, BackendSocket>;
using ClientsSocketConnnectionsMap =
    std::unordered_map<ClientId, ClientSocket *>;
using Url = std::string;

class WebSocketBridge {
public:
  explicit WebSocketBridge(Url backend_url);

  void onClientConnect(ClientSocket &client);
  void onClientMessage(ClientSocket &client, const std::string &data);
  void onClientClose(ClientSocket &client, const std::string &reason,
                     uint16_t code);

private:
  ClientsSocketConnnectionsMap clients_;
  BackendSocketConnectionsMap connections_;
  Url backend_url_;

  BackendSocket createBackendConnection(const ClientId &client_id);
};
