#include "RealSocket.h"

RealSocket::RealSocket(QWebSocket *socket) : socket_(socket) {
  connect(socket_, &QWebSocket::connected, this, &ISocket::connected);
  connect(socket_, &QWebSocket::textMessageReceived, this, &ISocket::textMessageReceived);
}

void RealSocket::open(const QUrl &url) { socket_->open(url); }

void RealSocket::sendTextMessage(const QString &msg) {
  LOG_INFO("Text to send in socket {}", msg.toStdString());
  socket_->sendTextMessage(msg);
}

void RealSocket::close() { socket_->close(); }

void RealSocket::disconnectSocket() { socket_->close(); }
