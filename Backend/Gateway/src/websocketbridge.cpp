#include "websocketbridge.h"
#include "Debug_profiling.h"

namespace{

std::string makeClientId(crow::websocket::connection& client) {
  return client.get_remote_ip() + ":" + std::to_string((uintptr_t)&client);
}

} // namespace

WebSocketBridge::WebSocketBridge(const std::string& backend_url) : backend_url_(backend_url) {}

std::shared_ptr<ix::WebSocket> WebSocketBridge::createBackendConnection(
    const std::string& client_id) {
  auto backend_ws = std::make_shared<ix::WebSocket>();
  backend_ws->setUrl(backend_url_);

  backend_ws->setOnMessageCallback([this, client_id](const ix::WebSocketMessagePtr& msg) {
    switch (msg->type) {
      case ix::WebSocketMessageType::Open:
        LOG_INFO("Backend WS connected");
        break;
      case ix::WebSocketMessageType::Message:
        clients_.at(client_id)->send_text(msg->str);
        break;
      case ix::WebSocketMessageType::Close:
        LOG_INFO(
            "Backend WS closed: code={}, reason={}", msg->closeInfo.code, msg->closeInfo.reason);
        break;
      case ix::WebSocketMessageType::Error:
        break;
      default:
        break;
    }
  });

  backend_ws->start();
  return backend_ws;
}

void WebSocketBridge::onClientConnect(crow::websocket::connection& client) {
  std::string client_id = makeClientId(client);
  auto backend_connection = createBackendConnection(client_id);
  clients_[client_id] =  &client;
  connections_[client_id] = backend_connection;
}

void WebSocketBridge::onClientMessage(crow::websocket::connection& client,
                                      const std::string&           data) {
  std::string client_id = makeClientId(client);
  auto it = connections_.find(client_id);
  if (it != connections_.end() && it->second) {
    it->second->send(data);
  }
}

void WebSocketBridge::onClientClose(crow::websocket::connection& client,
                                    const std::string& reason,
                                    uint16_t code) {
  std::string id = makeClientId(client);

  auto it = connections_.find(id);
  if (it != connections_.end()) {
    if (it->second && it->second->getReadyState() == ix::ReadyState::Open)
      it->second->close();
    connections_.erase(it);
  }

  clients_.erase(id);
}
