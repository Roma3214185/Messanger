#include "managers/socketmanager.h"

#include <QJsonArray>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "interfaces/ISocket.h"

SocketManager::SocketManager(ISocket *socket, const QUrl &url) : socket_(socket), url_(url) {
  url_.setScheme("ws");
  url_.setPath("/ws");
  connect(socket_, &ISocket::textMessageReceived, this, &SocketManager::newTextFromSocket);
}

void SocketManager::initSocket(long long user_id) {
  QJsonObject json{{"type", "init"}, {"user_id", user_id}};
  const QString msg = QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
  socket_->sendTextMessage(msg);
  LOG_INFO("[onSocketConnected] WebSocket initialized for userId={}", user_id);
}

void SocketManager::connectSocket() { socket_->open(url_); }

void SocketManager::close() {
  socket_->disconnectSocket();
  socket_->close();
}

void SocketManager::sendText(const QString &message) { socket_->sendTextMessage(message); }
