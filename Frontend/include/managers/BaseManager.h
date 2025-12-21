#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QFuture>
#include <QNetworkReply>
#include <QObject>
#include <QPromise>
#include <QTimer>
#include <QUrl>
#include <QJsonArray>
#include <QJsonObject>

#include "interfaces/INetworkAccessManager.h"
#include "Debug_profiling.h"

class BaseManager : public QObject {
  Q_OBJECT
 public:
  explicit BaseManager(INetworkAccessManager*     network_manager,
                       const QUrl&                base_url,
                       std::chrono::milliseconds  timeout_ms = std::chrono::milliseconds{ 500 },
                       QObject*                    arent     = nullptr);
  virtual ~BaseManager();

 protected:
  template <typename T, typename Callback>
  QFuture<T> handleReplyWithTimeout(QNetworkReply*            reply,
                                    Callback                  onSuccess,
                                    std::chrono::milliseconds timeout_ms,
                                    const T&       defaultValue = T()) {
    auto promisePtr  = std::make_shared<QPromise<T>>();
    auto future      = promisePtr->future();
    auto isCompleted = std::make_shared<std::atomic_bool>(false);

    QTimer::singleShot(timeout_ms, reply, [reply, promisePtr, isCompleted, this, defaultValue]() mutable {
      if (isCompleted->exchange(true)) return;

      if (reply->isRunning()) {
        reply->abort();
        Q_EMIT errorOccurred(kServerNotRespondError);
        promisePtr->addResult(defaultValue);
        promisePtr->finish();
      }
    });

    QObject::connect(reply,
                     &QNetworkReply::finished,
                     reply,
                     [reply, promisePtr, isCompleted, onSuccess, defaultValue, this]() mutable {
                       if (isCompleted->exchange(true)) {
                         reply->deleteLater();
                         return;
                       }

                       if (reply->error() != QNetworkReply::NoError) {
                         Q_EMIT errorOccurred(kErrorOccured + reply->errorString());
                         promisePtr->addResult(defaultValue);
                         return;
                       }

                       int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                       if(httpStatus == 202) {
                         std::string task_id = extractTaskId(reply);
                         QByteArray new_array = getRequestStatus(task_id);
                         promisePtr->addResult(onSuccess(new_array));
                        } else {
                          promisePtr->addResult(onSuccess(reply->readAll()));
                        }
                        promisePtr->finish();
                        reply->deleteLater();
                      });

    return future;
  }

  QFuture<void> handleReplyWithTimeoutVoid(QNetworkReply*                      reply,
                                           std::function<void(const QByteArray&)> onFinished,
                                           std::chrono::milliseconds           timeout_ms) {
    auto promisePtr  = std::make_shared<QPromise<void>>();
    auto future      = promisePtr->future();
    auto isCompleted = std::make_shared<std::atomic_bool>(false);

    QTimer::singleShot(timeout_ms, reply, [reply, promisePtr, isCompleted, this]() {
      if (isCompleted->exchange(true)) return;

      if (reply->isRunning()) {
        Q_EMIT errorOccurred(kServerNotRespondError);
        reply->abort();
        promisePtr->finish();
      }
    });

    QObject::connect(
        reply, &QNetworkReply::finished, reply, [this, reply, promisePtr, isCompleted, onFinished]() mutable {
          if (isCompleted->exchange(true)) {
            reply->deleteLater();
            return;
          }

          //TODO: if(!checkReply(reply) {

          int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
          if(httpStatus == 202) {
            std::string task_id = extractTaskId(reply);
            QByteArray new_array = getRequestStatus(task_id);
            onFinished(new_array);
          } else {
            onFinished(reply->readAll());
          }

          promisePtr->finish();
          reply->deleteLater();
        });

    return future;
  }

  QByteArray getRequestStatus(const std::string& task_id, int attempts = 5) {
    LOG_INFO("Get request status for task with id {}", task_id);

    QString path = QString("/request/%1/status").arg(QString::fromStdString(task_id));
    QUrl endpoint = url_.resolved(QUrl(path));

    LOG_INFO("Url for sending: {}", endpoint.toString().toStdString());

    for (int i = 1; i <= attempts; i++) {
      std::this_thread::sleep_for(timeout_ms_);
      LOG_INFO("Attempt #{}", i);

      QNetworkRequest request(endpoint);
      request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

      QNetworkReply* reply = network_manager_->get(request);

      QEventLoop loop;
      QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit); //TODO: "wait for" function
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
    return QByteArray();
  }

  bool checkReply(QNetworkReply*);

  std::string extractTaskId(QNetworkReply* reply) {
    QByteArray raw = reply->readAll();
    auto doc = QJsonDocument::fromJson(raw);
    if (!doc.isObject() || !doc.object().contains("request_id")) {
      LOG_ERROR("Reply don't have request_id field, skip");
      return "";
    }

    return doc["request_id"].toString().toStdString();
  }

 protected:
  INetworkAccessManager*    network_manager_;
  QUrl                      url_;
  std::chrono::milliseconds timeout_ms_;

 private:
  const QString kServerNotRespondError = "Server didn't respond";
  const QString kErrorOccured          = "Error occurred: ";
  const QString kUnknownError          = "UnknowError";

 Q_SIGNALS:
  void errorOccurred(const QString& message) const;
};

#endif  // BASEMANAGER_H
