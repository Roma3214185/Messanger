#ifndef FAKESOCKET_H
#define FAKESOCKET_H

#include <QObject>

#include "interfaces/ISocket.h"

class FakeSocket : public ISocket {
 public:
  void open(const QUrl& url) override {
    ++open_calls;
    last_opened_url = url;
  }

  void sendTextMessage(const QString& msg) override {
    ++sendTextMessage_calls;
    last_sended_message = msg;
  }

  void close() override {

  }

  void receiveTextMessage(const QString& message){
    Q_EMIT textMessageReceived(message);
  }

  int open_calls = 0;
  int sendTextMessage_calls = 0;
  QUrl last_opened_url;
  QString last_sended_message;
};

#endif  // FAKESOCKET_H
