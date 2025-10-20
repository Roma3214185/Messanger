#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QUrl>
#include <QWebSocket>
#include <unordered_map>
#include <memory>

#include "headers/SignUpRequest.h"
#include "headers/User.h"
#include "headers/INetworkAccessManager.h"
#include "headers/ICache.h"

class ChatBase;
class MessageModel;
class ChatModel;
class UserModel;
class Message;
class QNetworkReply;

using ChatId = int;
using std::optional;
using std::shared_ptr;
using std::unique_ptr;
using std::string;
using ChatPtr = std::shared_ptr<ChatBase>;
using MessageModelPtr = std::shared_ptr<MessageModel>;
using ChatMap = std::unordered_map<ChatId, ChatPtr>;
using MessageModelMap = std::unordered_map<ChatId, MessageModelPtr>;

class Model : public QObject
{
    Q_OBJECT

public:

    Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash, QWebSocket* socket);

    ChatModel* getChatModel();
    UserModel* getUserModel();
    MessageModelPtr createMessageModel(int chatId);
    MessageModel* getMessageModel(int chatId);
    void checkToken();
    void signIn(const QString& email, const QString& password);
    void signUp(const SignUpRequest& req);
    void connectSocket(const int id);
    ChatPtr loadChat(const int chatId);
    QList<User> findUsers(const QString& text);
    optional<User> getUser(const int userId);
    ChatPtr createPrivateChat(int userId);
    QList<Message> getChatMessages(int chatId, int limit = 20);
    void sendMessage(const int chatId, const int senderId, const QString& textToSend);
    QList<ChatPtr> loadChats();
    void signMe(const QString& token);
    void fillChatHistory(int chatId);
    void addChat(const ChatPtr& chat);
    void addChatInFront(const ChatPtr& chat);
    void createChat(const int chatId);
    void addMessageToChat(int chatId, const Message& msg, bool infront = true);
    void deleteToken() const;
    ChatPtr getPrivateChatWithUser(int userId);
    void saveToken(const QString& token) const;
    void clearAllChats();
    void clearAllMessages();
    void logout();
    int getNumberOfExistingChats() const;
    QModelIndex indexByChatId(int chatId) const;
    void setCurrentId(int id);

Q_SIGNALS:

    void chatAdded(const int id);
    void errorOccurred(const QString& error);
    void userCreated(const User& user, const QString& token);
    void newMessage(Message& message);
    void chatUpdated(int chatId);

private:

    void onSignInFinished(QNetworkReply* reply);
    void onSignUpFinished(QNetworkReply* reply);
    void onMessageReceived(const QString& msg);
    void onSocketConnected(int id);
    ChatPtr onChatLoaded(QNetworkReply* reply);
    QList<User> onFindUsers(QNetworkReply* reply);
    ChatPtr onCreatePrivateChat(QNetworkReply* reply);
    QList<Message> onGetChatMessages(QNetworkReply* reply);
    QList<ChatPtr> onLoadChats(QNetworkReply* reply);
    void onSignMe(QNetworkReply* reply);
    optional<User> onGetUser(QNetworkReply* reply);
    QNetworkRequest getRequestWithToken(QUrl endpoint);

    QUrl url_;
    INetworkAccessManager* netManager;
    ICache* cash;
    QWebSocket* socket;
    QString currentToken;
    unique_ptr<ChatModel> chatModel;
    unique_ptr<UserModel> userModel;
    ChatMap chatsById;
    MessageModelMap messageModelsByChatId;
};

#endif // MODEL_H
