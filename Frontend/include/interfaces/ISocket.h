#ifndef ISOCKET_H
#define ISOCKET_H

#include <QObject>
#include <QUrl>

class ISocket : public QObject {
  Q_OBJECT

 public:
  virtual ~ISocket()                               = default;
  virtual void open(const QUrl& url)               = 0;
  virtual void sendTextMessage(const QString& msg) = 0;
  virtual void close()                             = 0;

 Q_SIGNALS:
  void connected();
  void textMessageReceived(const QString& message);
};

#endif  // ISOCKET_H
