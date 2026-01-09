#include "managers/networkaccessmanager.h"

QNetworkReply *NetworkAccessManager::post(const QNetworkRequest &req, const QByteArray &byte_array) {
  return net_manager_.post(req, byte_array);
}

QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &req) { return net_manager_.get(req); }

QNetworkReply *NetworkAccessManager::put(const QNetworkRequest &req, const QByteArray &byte_array) {
  return net_manager_.put(req, byte_array);
}

QNetworkReply *NetworkAccessManager::del(const QNetworkRequest &req) { return net_manager_.deleteResource(req); }
