#include "websocketbridge.h"

#include "Debug_profiling.h"

namespace {

std::string makeClientId(crow::websocket::connection &client) {
  return client.get_remote_ip() + ":" + std::to_string(reinterpret_cast<uintptr_t>(&client));
}

}  // namespace

WebSocketBridge::WebSocketBridge(std::string backend_url) : backend_url_(std::move(backend_url)) {}

std::shared_ptr<ix::WebSocket> WebSocketBridge::createBackendConnection(const std::string &client_id) {
  auto backend_ws = std::make_shared<ix::WebSocket>();
  backend_ws->setUrl(backend_url_);

  using enum ix::WebSocketMessageType;
  backend_ws->setOnMessageCallback([this, client_id](const ix::WebSocketMessagePtr &msg) noexcept {
    try {
      switch (msg->type) {
        case Open:
          LOG_INFO("Backend WS connected");
          break;
        case Message: {
          auto it = clients_.find(client_id);
          if (it != clients_.end()) {
            it->second->send_text(msg->str);
          } else {
            LOG_WARN("Client {} not found", client_id);
          }
          break;
        }
        case Close:
          LOG_INFO("Backend WS closed: code={}, reason={}", msg->closeInfo.code, msg->closeInfo.reason);
          break;
        case Error:
          LOG_ERROR("Backend WS error occurred");
          break;
        default:
          break;
      }
    } catch (const std::exception &e) {
      LOG_ERROR("Exception in WS callback: {}", e.what());
    } catch (...) {
      LOG_ERROR("Unknown exception in WS callback");
    }
  });

  backend_ws->start();
  return backend_ws;
}

void WebSocketBridge::onClientConnect(crow::websocket::connection &client) {
  std::string client_id = makeClientId(client);
  auto backend_connection = createBackendConnection(client_id);
  clients_[client_id] = &client;
  connections_[client_id] = backend_connection;
}

void WebSocketBridge::onClientMessage(crow::websocket::connection &client, const std::string &data) {
  const std::string client_id = makeClientId(client);
  if (auto it = connections_.find(client_id); it != connections_.end() && it->second) {
    it->second->send(data);
  }
}

void WebSocketBridge::onClientClose(crow::websocket::connection &client, const std::string & /*reason*/,
                                    uint16_t /*code*/) {
  const std::string id = makeClientId(client);
  if (auto it = connections_.find(id); it != connections_.end()) {
    if (it->second && it->second->getReadyState() == ix::ReadyState::Open) {
      it->second->close();
    }
    connections_.erase(it);
  }

  clients_.erase(id);
}
