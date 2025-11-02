#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QUrl>

class INetworkAccessManager;
class LogInRequest;
class QNetworkReply;
class User;
class SignUpRequest;

class SessionManager : public QObject {
    Q_OBJECT
  public:
    SessionManager(INetworkAccessManager* net_manager, const QUrl& url);
    void signIn(const LogInRequest& login_request);
    void signUp(const SignUpRequest& signup_request);
    void authenticateWithToken(const QString& token);

  protected:
    virtual void onReplyFinished(QNetworkReply* reply);

  private:
    INetworkAccessManager* network_manager_;
    QUrl url_;

  Q_SIGNALS:
    void errorOccurred(const QString& error);
    void userCreated(const User& user, const QString& token);
};

#endif // SESSIONMANAGER_H
