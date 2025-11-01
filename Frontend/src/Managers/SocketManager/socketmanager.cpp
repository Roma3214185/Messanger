#include "socketmanager.h"

#include <QJsonArray>

#include "DebugProfiling/Debug_profiling.h"
#include "headers/JsonService.h"

SocketManager::SocketManager(QWebSocket* socket, QUrl url)
    : socket_(socket)
    , url_(url) {}

void SocketManager::onSocketConnected(int user_id) {
  PROFILE_SCOPE("Model::onSocketConnected");
  QJsonObject json{{"type", "init"}, {"userId", user_id}};
  const QString msg =
      QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
  socket_->sendTextMessage(msg);
  LOG_INFO("[onSocketConnected] WebSocket initialized for userId={}", user_id);
}

void SocketManager::connectSocket(int user_id) {
  PROFILE_SCOPE("Model::connectSocket");
  connect(socket_, &QWebSocket::connected,
        [this, user_id]() -> void { onSocketConnected(user_id); });
  connect(socket_, &QWebSocket::textMessageReceived, this,
        &SocketManager::newTextFromSocket);
  socket_->open(QUrl("ws://localhost:8086/ws"));
  LOG_INFO("[connectSocket] Connecting WebSocket for userId={}", user_id);
}

void SocketManager::close(){
  socket_->disconnect();
  socket_->close();
}

void SocketManager::sendText(const QString &message) {
  socket_->sendTextMessage(message);
}
