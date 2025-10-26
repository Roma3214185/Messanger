#ifndef INETWORKACCESSMANAGER_H
#define INETWORKACCESSMANAGER_H

#include <QNetworkReply>

class INetworkAccessManager {
 public:
  virtual auto post(const QNetworkRequest&, const QByteArray&) -> QNetworkReply* = 0;
  virtual auto get(const QNetworkRequest&) -> QNetworkReply* = 0;
};

#endif  // INETWORKACCESSMANAGER_H
