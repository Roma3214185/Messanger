#include "BaseManager.h"

#include <QFuture>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QPromise>
#include <QTimer>
#include <QUrl>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "interfaces/INetworkAccessManager.h"


template <typename T, typename Callback>
QFuture<T> BaseManager::handleReplyWithTimeout(QNetworkReply *reply, Callback on_success, std::chrono::milliseconds timeout_ms,
                                  const T &default_value) {
    auto promise_ptr = std::make_shared<QPromise<T>>();
    auto future = promise_ptr->future();
    auto is_completed = std::make_shared<std::atomic_bool>(false);

    QTimer::singleShot(timeout_ms, reply, [reply, promise_ptr, is_completed, this, default_value]() mutable {
        if (is_completed->exchange(true)) return;

        if (reply->isRunning()) {
            reply->abort();
            Q_EMIT errorOccurred(kServerNotRespondError);
            promise_ptr->addResult(default_value);
            promise_ptr->finish();
        }
    });

    QObject::connect(reply, &QNetworkReply::finished, reply,
                     [reply, promise_ptr, is_completed, on_success, default_value, this]() mutable {
                         if (is_completed->exchange(true)) {
                             reply->deleteLater();
                             return;
                         }

                         if (reply->error() != QNetworkReply::NoError) {
                             Q_EMIT errorOccurred(kErrorOccured + reply->errorString());
                             promise_ptr->addResult(default_value);
                             return;
                         }

                         const int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                         LOG_INFO("httpStatus = ", http_status);
                         if (http_status == 202) {
                             std::string task_id = extractTaskId(reply);
                             QByteArray new_array = getRequestStatus(task_id);
                             promise_ptr->addResult(on_success(new_array));
                         } else {
                             promise_ptr->addResult(on_success(reply->readAll()));
                         }
                         promise_ptr->finish();
                         reply->deleteLater();
                     });

    return future;
}


