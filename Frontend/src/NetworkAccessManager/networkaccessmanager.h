#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

#include "headers/INetworkAccessManager.h"

class NetworkAccessManager : public INetworkAccessManager {
  QNetworkAccessManager net_manager_;

 public:
  QNetworkReply* post(const QNetworkRequest& req,
                      const QByteArray& byteArray) override;
  QNetworkReply* get(const QNetworkRequest& req) override;
};

#endif  // NETWORKACCESSMANAGER_H
