#include "managers/BaseManager.h"

BaseManager::BaseManager(INetworkAccessManager *network_manager, const QUrl &base_url, EntityFactory *entity_factory,
                         std::chrono::milliseconds timeout_ms, QObject *parent)
    : QObject(parent),
      network_manager_(network_manager),
      url_(base_url),
      entity_factory_(entity_factory),
      timeout_ms_(timeout_ms) {}

BaseManager::~BaseManager() = default;

bool BaseManager::checkReply(QNetworkReply *reply) {
  if (!reply || reply->error() != QNetworkReply::NoError) {
    if (reply)
      LOG_ERROR("[onReplyFinished] Network error: '{}'", reply->errorString().toStdString());
    else
      LOG_ERROR("Reply is nullptr");
    Q_EMIT errorOccurred("Reply is failed");
    return false;
  }

  return true;
}
