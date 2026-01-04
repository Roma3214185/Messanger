#ifndef INETWORKACCESSMANAGER_H
#define INETWORKACCESSMANAGER_H

#include <QNetworkReply>

class INetworkAccessManager {
 public:
  virtual auto post(const QNetworkRequest&, const QByteArray&) -> QNetworkReply*           = 0;
  virtual auto get(const QNetworkRequest&) -> QNetworkReply*                               = 0;
  virtual auto put(const QNetworkRequest&, const QByteArray& byte_array) -> QNetworkReply* = 0;
  virtual auto del(const QNetworkRequest&) -> QNetworkReply*                               = 0;
  virtual ~INetworkAccessManager() = default;
};

#endif  // INETWORKACCESSMANAGER_H
