#include "networkaccessmanager.h"

QNetworkReply* NetworkAccessManager::post(const QNetworkRequest& req,
                                          const QByteArray& byteArray) {
  qDebug() << "Reall post";
  return net_manager_.post(req, byteArray);
}

QNetworkReply* NetworkAccessManager::get(const QNetworkRequest& req) {
  qDebug() << "Reall get";
  return net_manager_.get(req);
}
