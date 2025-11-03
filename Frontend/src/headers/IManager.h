#ifndef IMANAGER_H
#define IMANAGER_H

#include <QObject>
#include <QFuture>
#include <QNetworkReply>
#include <QPromise>
#include <QTimer>

class IManager : public QObject {
    Q_OBJECT
  public:
    virtual ~IManager() = default;

  protected:
    template<typename T, typename Callback>
    QFuture<T> handleReplyWithTimeout(
        QNetworkReply* reply,
        Callback onSuccess,
        int timeout_ms,
        const T& defaultValue = T())
    {
      auto promisePtr = std::make_shared<QPromise<T>>();
      auto future = promisePtr->future();
      auto isCompleted = std::make_shared<std::atomic_bool>(false);

      QTimer::singleShot(timeout_ms, reply, [reply, promisePtr, isCompleted, this, defaultValue]() {
        if (isCompleted->exchange(true))
          return;

        if (reply->isRunning()) {
          reply->abort();
          Q_EMIT errorOccurred(kServerNotRespondError);
          promisePtr->addResult(defaultValue);
          promisePtr->finish();
        }
      });

      QObject::connect(reply, &QNetworkReply::finished, reply,
                       [reply, promisePtr, isCompleted, onSuccess, defaultValue, this]() mutable {
        if (isCompleted->exchange(true)) {
          reply->deleteLater();
          return;
        }

        if (reply->error() != QNetworkReply::NoError) {
          Q_EMIT errorOccurred(kErrorOccured + reply->errorString());
          promisePtr->addResult(defaultValue);
        } else {
          promisePtr->addResult(onSuccess(reply));
        }

        promisePtr->finish();
        reply->deleteLater();
      });

      return future;
    }

    QFuture<void> handleReplyWithTimeoutVoid(
        QNetworkReply* reply,
        std::function<void(QNetworkReply*)> onFinished,
        int timeout_ms)
    {
      auto promisePtr = std::make_shared<QPromise<void>>();
      auto future = promisePtr->future();
      auto isCompleted = std::make_shared<std::atomic_bool>(false);

      QTimer::singleShot(timeout_ms, reply, [reply, promisePtr, isCompleted, this]() {
        if (isCompleted->exchange(true))
          return;

        if (reply->isRunning()) {
          Q_EMIT errorOccurred(kServerNotRespondError);
          reply->abort();
          promisePtr->finish();
        }
      });

      QObject::connect(reply, &QNetworkReply::finished, reply, [reply, promisePtr, isCompleted, onFinished]() {
        if (isCompleted->exchange(true)) {
          reply->deleteLater();
          return;
        }

        onFinished(reply);

        promisePtr->finish();
        reply->deleteLater();
      });

      return future;
    }

  private:
    const QString kServerNotRespondError = "Server didn't respond";
    const QString kErrorOccured = "Error occurred: ";
    const QString kUnknownError = "UnknowError";

  Q_SIGNALS:
    void errorOccurred(const QString& message);
};

#endif // IMANAGER_H
