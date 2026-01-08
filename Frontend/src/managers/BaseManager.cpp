#include "managers/BaseManager.h"

BaseManager::BaseManager(INetworkAccessManager *network_manager,
                         const QUrl &base_url,
                         std::chrono::milliseconds timeout_ms, QObject *parent)
    : QObject(parent), network_manager_(network_manager), url_(base_url),
      timeout_ms_(timeout_ms) {}

BaseManager::~BaseManager() = default;

bool BaseManager::checkReply(QNetworkReply *reply) {
  if (!reply || reply->error() != QNetworkReply::NoError) {
    if (reply)
      LOG_ERROR("[onReplyFinished] Network error: '{}'",
                reply->errorString().toStdString());
    else
      LOG_ERROR("Reply is nullptr");
    Q_EMIT errorOccurred("Reply is failed");
    return false;
  }

  return true;
}
