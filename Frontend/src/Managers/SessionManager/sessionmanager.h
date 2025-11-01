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
    INetworkAccessManager* net_manager_;
    QUrl url_;

  public:
    SessionManager(INetworkAccessManager* net_manager, QUrl url);
    void signIn(const LogInRequest& login_request);
    void signUp(const SignUpRequest& signup_request);
    void authenticateWithToken(const QString& token);

  private:
    void onSignInFinished(QNetworkReply* reply);
    void onSignUpFinished(QNetworkReply* reply);
    void onAuthenticate(QNetworkReply* reply);

  Q_SIGNALS:
    void errorOccurred(const QString& error);
    void userCreated(const User& user, const QString& token);
};

#endif // SESSIONMANAGER_H
