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
#include "JsonService.h"
#include "interfaces/INetworkAccessManager.h"

class BaseManager : public QObject {
  Q_OBJECT
 public:
  BaseManager(INetworkAccessManager *network_manager, const QUrl &base_url, std::chrono::milliseconds timeout_ms,
              QObject *parent);
  ~BaseManager() override;

 protected:
  template <typename T, typename Callback>
  QFuture<T> handleReplyWithTimeout(QNetworkReply *reply, Callback on_success, std::chrono::milliseconds timeout_ms,
                                       const T &default_value = T());

  using OnFinishedCallback = const std::function<void(const QByteArray &)> &;
  QFuture<void> handleReplyWithTimeoutVoid(QNetworkReply *reply,
                                           OnFinishedCallback on_finished,
                                           std::chrono::milliseconds timeout_ms);

  QByteArray getRequestStatus(const std::string &task_id, int attempts = 5);
  bool checkReply(QNetworkReply *);
  static std::string extractTaskId(QNetworkReply *reply);
  static QNetworkRequest getRequestWithToken(const QUrl &endpoint, const QString &current_token);

  INetworkAccessManager *network_manager_;
  QUrl url_;
  std::chrono::milliseconds timeout_ms_;
  const QString kServerNotRespondError = "Server didn't respond";
  const QString kErrorOccured = "Error occurred: ";
  const QString kUnknownError = "UnknownError";

 Q_SIGNALS:
  void errorOccurred(const QString &message) const;
};

#include "BaseManager.inl"

#endif  // BASEMANAGER_H
