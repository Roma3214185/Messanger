#include "managers/networkaccessmanager.h"

QNetworkReply* NetworkAccessManager::post(const QNetworkRequest& req, const QByteArray& byteArray) {
  return net_manager_.post(req, byteArray);
}

QNetworkReply* NetworkAccessManager::get(const QNetworkRequest& req) {
  return net_manager_.get(req);
}
