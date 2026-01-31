#ifndef REALSOCKET_H
#define REALSOCKET_H

#include <QWebSocket>

#include "Debug_profiling.h"
#include "interfaces/ISocket.h"

class RealSocket : public ISocket {
  QWebSocket *socket_;

 public:
  explicit RealSocket(QWebSocket *socket);

  void open(const QUrl &url) override;
  void sendTextMessage(const QString &msg) override;
  void close() override;
  void disconnectSocket() override;
};

#endif  // REALSOCKET_H
