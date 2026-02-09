#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QFuture>
#include <QObject>
#include <QUrl>

class INetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class BaseManager : public QObject {
  Q_OBJECT
 public:
  using Timeout = std::chrono::milliseconds;
  using OnFinishedCallback = const std::function<void(const QByteArray &)> &;

  BaseManager(INetworkAccessManager *network_manager, const QUrl &base_url, Timeout timeout_ms, QObject *parent);
  ~BaseManager() override;

 protected:
  template <typename T, typename Callback>
  QFuture<T> handleReplyWithTimeout(QNetworkReply *reply, Callback on_success, Timeout timeout_ms,
                                    const T &default_value = T());

  QFuture<void> handleReplyWithTimeoutVoid(QNetworkReply *reply, OnFinishedCallback on_finished, Timeout timeout_ms);

  QByteArray getRequestStatus(const std::string &task_id, int attempts = 5);
  bool checkReply(QNetworkReply *);
  static std::string extractTaskId(QNetworkReply *reply);
  static QNetworkRequest getRequestWithToken(const QUrl &endpoint, const QString &current_token);

  INetworkAccessManager *network_manager_;
  QUrl url_;
  Timeout timeout_ms_;
  const QString kServerNotRespondError = "Server didn't respond";
  const QString kErrorOccured = "Error occurred: ";
  const QString kUnknownError = "UnknownError";

 Q_SIGNALS:
  void errorOccurred(const QString &message) const;
};

#include "BaseManager.inl"

#endif  // BASEMANAGER_H
