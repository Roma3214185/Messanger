#include "managers/BaseManager.h"

BaseManager::BaseManager(INetworkAccessManager *network_manager, const QUrl &base_url,
                         std::chrono::milliseconds timeout_ms, QObject *parent)
    : QObject(parent), network_manager_(network_manager), url_(base_url), timeout_ms_(timeout_ms) {}

BaseManager::~BaseManager() = default;

bool BaseManager::checkReply(QNetworkReply *reply) {
  if (!reply || reply->error() != QNetworkReply::NoError) {
    if (reply) {
      LOG_ERROR("[onReplyFinished] Network error: '{}'", reply->errorString().toStdString());
    } else {
      LOG_ERROR("Reply is nullptr");
    }
    Q_EMIT errorOccurred("Reply is failed");
    return false;
  }

  return true;
}


QFuture<void> BaseManager::handleReplyWithTimeoutVoid(QNetworkReply *reply,
                                         OnFinishedCallback on_finished,
                                         std::chrono::milliseconds timeout_ms) {
    auto promise_ptr = std::make_shared<QPromise<void>>();
    auto future = promise_ptr->future();
    auto is_completed = std::make_shared<std::atomic_bool>(false);

    QTimer::singleShot(timeout_ms, reply, [reply, promise_ptr, is_completed, this]() {
        if (is_completed->exchange(true)) return;

        if (reply->isRunning()) {
            Q_EMIT errorOccurred(kServerNotRespondError);
            reply->abort();
            promise_ptr->finish();
        }
    });

    QObject::connect(reply, &QNetworkReply::finished, reply,
                     [this, reply, promise_ptr, is_completed, on_finished]() mutable {
                         if (is_completed->exchange(true)) {
                             reply->deleteLater();
                             return;
                         }

                         // TODO(roma): if(!checkReply(reply) {

                         int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                         LOG_INFO("http_status = ", http_status);
                         if (http_status == 202) {
                             const std::string task_id = extractTaskId(reply);
                             on_finished(getRequestStatus(task_id));
                         } else {
                             on_finished(reply->readAll());
                         }

                         promise_ptr->finish();
                         reply->deleteLater();
                     });

    return future;
}

QByteArray BaseManager::getRequestStatus(const std::string &task_id, int attempts) {
    LOG_INFO("Get request status for task with id {}", task_id);
    const QString path = QString("/request/%1/status").arg(QString::fromStdString(task_id));
    const QUrl endpoint = url_.resolved(QUrl(path));
    LOG_INFO("Url for sending: {}", endpoint.toString().toStdString());

    for (int i = 1; i <= attempts; i++) {
        std::this_thread::sleep_for(timeout_ms_);
        LOG_INFO("Attempt #{}", i);

        QNetworkRequest request(endpoint);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = network_manager_->get(request);

        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop,
                         &QEventLoop::quit);  // TODO(roma): "wait for" function
        loop.exec();

        QByteArray raw = reply->readAll();
        LOG_INFO("Answer raw: {}", raw.toStdString());
        auto doc = QJsonDocument::fromJson(raw);

        if (!doc.isObject() || !doc.object().contains("status")) {
            LOG_ERROR("Reply doesn't have status field, skip");
            reply->deleteLater();
            continue;
        }

        QString status = doc["status"].toString();
        if (status != "finished") {
            LOG_INFO("Status not finished");
            reply->deleteLater();
            continue;
        }

        return raw;
    }

    LOG_INFO("All attempts failed");
    return {};
}

std::string BaseManager::extractTaskId(QNetworkReply *reply) {
    QByteArray raw = reply->readAll();
    auto doc = QJsonDocument::fromJson(raw);
    if (!doc.isObject() || !doc.object().contains("request_id")) {
        LOG_ERROR("Reply don't have request_id field, skip");
        return "";
    }

    return doc["request_id"].toString().toStdString();
}

QNetworkRequest BaseManager::getRequestWithToken(const QUrl &endpoint, const QString &current_token) {
    auto request = QNetworkRequest(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", current_token.toUtf8());
    return request;
}
