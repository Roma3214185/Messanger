#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QUrl>

#include "headers/IManager.h"

class INetworkAccessManager;
class LogInRequest;
class QNetworkReply;
class User;
class SignUpRequest;

class SessionManager : public IManager {
    Q_OBJECT
  public:
    SessionManager(INetworkAccessManager* net_manager, const QUrl& url, int timeout_ms = 5000);
    void signIn(const LogInRequest& login_request);
    void signUp(const SignUpRequest& signup_request);
    void authenticateWithToken(const QString& token);

  protected:
    virtual void onReplyFinished(QNetworkReply* reply);

  private:
    int timeout_ms_;
    INetworkAccessManager* network_manager_;
    QUrl url_;

  Q_SIGNALS:
    void userCreated(const User& user, const QString& token);
};

#endif // SESSIONMANAGER_H
