#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QUrl>
#include <QWebSocket>
#include <memory>
#include <unordered_map>

class ChatBase;
class MessageModel;
class ChatModel;
class UserModel;
class Message;
class QNetworkReply;
class INetworkAccessManager;
class ICache;
class SignUpRequest;
class LogInRequest;
class User;
class MessageInfo;

using ChatId = int;
using ChatPtr = std::shared_ptr<ChatBase>;
using MessageModelPtr = std::shared_ptr<MessageModel>;
using ChatMap = std::unordered_map<ChatId, ChatPtr>;
using MessageModelMap = std::unordered_map<ChatId, MessageModelPtr>;

class Model : public QObject {
  Q_OBJECT

 public:
  Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash,
        QWebSocket* socket);

  auto getChatModel() -> ChatModel*;
  auto getUserModel() -> UserModel*;
  auto createMessageModel(int chatId) -> MessageModelPtr;
  auto getMessageModel(int chatId) -> MessageModel*;
  void checkToken();
  void signIn(const LogInRequest& login_request);
  void signUp(const SignUpRequest& req);
  void connectSocket(int user_id);
  auto loadChat(int chatId) -> ChatPtr;
  auto findUsers(const QString& text) -> QList<User>;
  auto getUser(int userId) -> std::optional<User>;
  auto createPrivateChat(int userId) -> ChatPtr;
  auto getChatMessages(int chatId) -> QList<Message>;
  auto getChatMessages(int chatId, int limit) -> QList<Message>;
  void sendMessage(const MessageInfo& msg);
  auto loadChats() -> QList<ChatPtr>;
  void signMe(const QString& token);
  void fillChatHistory(int chatId);
  void addChat(const ChatPtr& chat);
  void addChatInFront(const ChatPtr& chat);
  void createChat(int chatId);
  void addMessageToChat(int chatId, const Message& msg);
  void addMessageToChat(int chatId, const Message& msg, bool infront);
  void deleteToken() const;
  auto getPrivateChatWithUser(int userId) -> ChatPtr;
  void saveToken(const QString& token) const;
  void clearAllChats();
  void clearAllMessages();
  void logout();
  [[nondiscard]] auto getNumberOfExistingChats() const -> int;
  [[nondiscard]] auto indexByChatId(int chat_id) -> QModelIndex;
  static void setCurrentId(int current_id);

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
  auto onChatLoaded(QNetworkReply* reply) -> ChatPtr;
  auto onFindUsers(QNetworkReply* reply) -> QList<User>;
  auto onCreatePrivateChat(QNetworkReply* reply) -> ChatPtr;
  auto onGetChatMessages(QNetworkReply* reply) -> QList<Message>;
  auto onLoadChats(QNetworkReply* reply) -> QList<ChatPtr>;
  void onSignMe(QNetworkReply* reply);
  auto onGetUser(QNetworkReply* reply) -> std::optional<User>;
  auto getRequestWithToken(QUrl endpoint) -> QNetworkRequest;

  QUrl url_;
  INetworkAccessManager* netManager;
  ICache* cash;
  QWebSocket* socket;
  QString currentToken;
  std::unique_ptr<ChatModel> chatModel;
  std::unique_ptr<UserModel> userModel;
  ChatMap chatsById;
  MessageModelMap messageModelsByChatId;
};

#endif  // MODEL_H
