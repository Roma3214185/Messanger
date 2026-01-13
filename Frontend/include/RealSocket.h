#ifndef REALSOCKET_H
#define REALSOCKET_H

#include <QWebSocket>

#include "Debug_profiling.h"
#include "interfaces/ISocket.h"

class RealSocket : public ISocket {
  QWebSocket *socket_;

 public:
  explicit RealSocket(QWebSocket *socket) : socket_(socket) {
    connect(socket_, &QWebSocket::connected, this, &ISocket::connected);
    connect(socket_, &QWebSocket::textMessageReceived, this, &ISocket::textMessageReceived);
  }

  void open(const QUrl &url) override { socket_->open(url); }
  void sendTextMessage(const QString &msg) override {
    LOG_INFO("Text to send in socket {}", msg.toStdString());
    socket_->sendTextMessage(msg);
  }
  void close() override { socket_->close(); }
  void disconnectSocket() override { socket_->disconnect(); }
};

#endif  // REALSOCKET_H
