#include "WebSocketBridge.h"

std::shared_ptr<ix::WebSocket> WebSocketBridge::createBackendConnection(crow::websocket::connection& client) {
  auto backend_ws = std::make_shared<ix::WebSocket>();
  backend_ws->setUrl(backend_url_);

  backend_ws->setOnMessageCallback([&client](const ix::WebSocketMessagePtr& msg) {
    switch (msg->type) {
    case ix::WebSocketMessageType::Open:
      LOG_INFO("Backend WS connected");
      break;
    case ix::WebSocketMessageType::Message:
      client.send_text(msg->str);
      break;
    case ix::WebSocketMessageType::Close:
      LOG_INFO("Backend WS closed: code={}, reason={}", msg->closeInfo.code, msg->closeInfo.reason);
      break;
    case ix::WebSocketMessageType::Error:
      LOG_ERROR("Backend WS error: {} ({})", msg->errorInfo.reason, msg->errorInfo.http_status);
      break;
    default:
      break;
    }
  });

  backend_ws->start();
  return backend_ws;
}

void WebSocketBridge::onClientConnect(crow::websocket::connection& client) {
  LOG_INFO("Frontend WS connected: {}", client.get_remote_ip());
  connections_[&client] = createBackendConnection(client);
}

void WebSocketBridge::onClientMessage(crow::websocket::connection& client, const std::string& data) {
  auto it = connections_.find(&client);
  if (it != connections_.end() && it->second) {
    it->second->send(data);
  }
}

void WebSocketBridge::onClientClose(crow::websocket::connection& client, const std::string& reason, uint16_t code) {
  LOG_INFO("Frontend WS closed: reason='{}', code={}", reason, code);
  auto it = connections_.find(&client);
  if (it != connections_.end()) {
    auto backend_ws = it->second;
    if (backend_ws && backend_ws->getReadyState() == ix::ReadyState::Open) {
      backend_ws->close();
    }
    connections_.erase(it);
  }
}
