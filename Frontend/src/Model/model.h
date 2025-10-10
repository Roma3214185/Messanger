#ifndef MODEL_H
#define MODEL_H

#include <headers/SignUpRequest.h>
#include <QtNetwork/QNetworkAccessManager>
#include <QObject>
#include <headers/User.h>
#include <QWebSocket>
#include "ChatModel/chatmodel.h"
#include <headers/User.h>
#include "MessageModel/messagemodel.h"

#include "UserModel/UserModel.h"
#include "headers/INetworkAccessManager.h"
#include "headers/ICash.h"

using ChatId = int;

class MessageModel;
class Message;

class Model : public QObject
{
    Q_OBJECT

    INetworkAccessManager* netManager;
    QString currentToken;
    QUrl url_;
    QWebSocket* socket;
    ICash* cash;
    std::unordered_map<ChatId, std::shared_ptr<ChatBase>> existingChats;
    std::unordered_map<ChatId, std::shared_ptr<MessageModel>> idToMessageModel;
    std::unique_ptr<ChatModel> chatModel;
    std::unique_ptr<UserModel> userModel;

public:
    Model(QUrl url, INetworkAccessManager* netManager, ICash* cash, QWebSocket* socket);
    ChatModel* getChatModel();
    UserModel* getUserModel();
    std::shared_ptr<MessageModel> createMessageModel(int chatId);
    MessageModel* getMessageModel(int chatId);
    void checkToken();
    void signIn(QString email, QString password);
    void signUp(SignUpRequest req);
    void connectSocket(int id);
    std::shared_ptr<ChatBase> loadChat(int chatId);
    QList<User> findUsers(QString text);
    std::optional<User> getUser(int userId);
    std::shared_ptr<ChatBase> createPrivateChat(int userId);
    QList<Message> getChatMessages(int chatId);
    void sendMessage(int chatId, int sender_id, const QString& textToSend);
    QList<std::shared_ptr<ChatBase>> loadChats();
    void signMe(QString token);
    void fillChatHistory(int chatId);
    void addChat(std::shared_ptr<ChatBase> chat);
    void addChatInFront(std::shared_ptr<ChatBase> chat);
    void createChat(int chatId);
    void addMessageToChat(int chatId, Message msg);
    void deleteToken();
    std::shared_ptr<ChatBase> getPrivateChatWithUser(int userId);
    void saveToken(const QString& token);
    void clearAllChats();
    void clearAllMessages();
    void logout();
    int getNumberOfExistingChats();
private:
    void onSignInFinished(QNetworkReply* reply);
    void onSignUpFinished(QNetworkReply* reply);
    void onMessageReceived(const QString& msg);
    void onSocketConnected(int id);
    std::shared_ptr<ChatBase> onChatLoaded(QNetworkReply* reply);
    QList<User> onFindUsers(QNetworkReply* reply);
    std::shared_ptr<ChatBase> onCreatePrivateChat(QNetworkReply* reply);
    QList<Message> onGetChatMessages(QNetworkReply* reply);
    QList<std::shared_ptr<ChatBase>> onLoadChats(QNetworkReply* reply);
    void onSignMe(QNetworkReply* reply);
    std::optional<User> onGetUser(QNetworkReply* reply);

Q_SIGNALS:
    void chatAdded(int id);
    void errorOccurred(QString error);
    void userCreated(User user, QString token);
    void newMessage(Message message);

};

#endif // MODEL_H
