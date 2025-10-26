#include "networkaccessmanager.h"

QNetworkReply* NetworkAccessManager::post(const QNetworkRequest& req,
                                          const QByteArray& byteArray) {
  return netManager.post(req, byteArray);
}

QNetworkReply* NetworkAccessManager::get(const QNetworkRequest& req) {
  return netManager.get(req);
}
