#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QFuture>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QPromise>
#include <QTimer>
#include <QUrl>

#include "Debug_profiling.h"
#include "interfaces/INetworkAccessManager.h"
#include "JsonService.h"

class BaseManager : public QObject {
  Q_OBJECT
 public:
  BaseManager(INetworkAccessManager *network_manager, const QUrl &base_url, EntityFactory *enity_factory,
                       std::chrono::milliseconds timeout_ms = std::chrono::milliseconds{500}, QObject *parent = nullptr);
  virtual ~BaseManager();

 protected:
  template <typename T, typename Callback>
  QFuture<T> handleReplyWithTimeout(QNetworkReply *reply, Callback on_success, std::chrono::milliseconds timeout_ms,
                                    const T &default_value = T()) {
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

  QFuture<void> handleReplyWithTimeoutVoid(QNetworkReply *reply, std::function<void(const QByteArray &)> on_finished,
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

                       // TODO: if(!checkReply(reply) {

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

  QByteArray getRequestStatus(const std::string &task_id, int attempts = 5) {
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
                       &QEventLoop::quit);  // TODO: "wait for" function
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

  bool checkReply(QNetworkReply *);

  static std::string extractTaskId(QNetworkReply *reply) {
    QByteArray raw = reply->readAll();
    auto doc = QJsonDocument::fromJson(raw);
    if (!doc.isObject() || !doc.object().contains("request_id")) {
      LOG_ERROR("Reply don't have request_id field, skip");
      return "";
    }

    return doc["request_id"].toString().toStdString();
  }

  INetworkAccessManager *network_manager_;
  EntityFactory* entity_factory_;
  QUrl url_;
  std::chrono::milliseconds timeout_ms_;

 private:
  const QString kServerNotRespondError = "Server didn't respond";
  const QString kErrorOccured = "Error occurred: ";
  const QString kUnknownError = "UnknowError";

 Q_SIGNALS:
  void errorOccurred(const QString &message) const;
};

#endif  // BASEMANAGER_H
