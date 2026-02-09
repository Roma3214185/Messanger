#include "RealSocket.h"

#include <QWebSocket>

#include "Debug_profiling.h"

struct RealSocket::Impl {
  QWebSocket socket;
};

RealSocket::RealSocket() : impl_(std::make_unique<Impl>()) {}

RealSocket::~RealSocket() = default;

void RealSocket::open(const QUrl &url) {
  connect(&impl_->socket, &QWebSocket::connected, this, &ISocket::connected);
  connect(&impl_->socket, &QWebSocket::textMessageReceived, this, &ISocket::textMessageReceived);
  impl_->socket.open(url);
}

void RealSocket::sendTextMessage(const QString &msg) {
  LOG_INFO("Text to send in socket {}", msg.toStdString());
  impl_->socket.sendTextMessage(msg);
}

void RealSocket::close() { impl_->socket.close(); }

void RealSocket::disconnectSocket() { impl_->socket.disconnect(); }
