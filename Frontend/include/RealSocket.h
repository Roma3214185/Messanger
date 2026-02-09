#ifndef REALSOCKET_H
#define REALSOCKET_H

#include "interfaces/ISocket.h"

class RealSocket : public ISocket {
 public:
  RealSocket();
  ~RealSocket();

  void open(const QUrl &url) override;
  void sendTextMessage(const QString &msg) override;
  void close() override;
  void disconnectSocket() override;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif  // REALSOCKET_H
