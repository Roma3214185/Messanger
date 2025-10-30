#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QUrl>
#include <QWebSocket>
#include <memory>
#include <unordered_map>
#include <optional>

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
  explicit Model(const QUrl& url, INetworkAccessManager* net_manager, ICache* cache, QWebSocket* socket);

  ChatModel* getChatModel() const;
  UserModel* getUserModel() const;
  [[nodiscard]] MessageModelPtr createMessageModel(int chat_id);
  MessageModel* getMessageModel(int chat_id);

  void checkToken();
  void signIn(const LogInRequest& login_request);
  void signUp(const SignUpRequest& request);
  void connectSocket(int user_id);

  [[nodiscard]] ChatPtr loadChat(int chat_id);
  [[nodiscard]] QList<User> findUsers(const QString& text);
  [[nodiscard]] std::optional<User> getUser(int user_id);
  ChatPtr createPrivateChat(int user_id);

  [[nodiscard]] QList<Message> getChatMessages(int chat_id);
  [[nodiscard]] QList<Message> getChatMessages(int chat_id, int limit);

  void sendMessage(const Message& msg);
  [[nodiscard]] QList<ChatPtr> loadChats();
  void authenticateWithToken(const QString& token);
  void fillChatHistory(int chat_id);
  void addChat(const ChatPtr& chat);
  void addChatInFront(const ChatPtr& chat);
  void createChat(int chat_id);
  void addMessageToChat(int chat_id, const Message& msg, bool in_front = false);
  void deleteToken() const;
  [[nodiscard]] ChatPtr getPrivateChatWithUser(int user_id);
  void saveToken(const QString& token) const;
  void clearAllChats();
  void clearAllMessages();
  void logout();

  [[nodiscard]] int getNumberOfExistingChats() const;
  [[nodiscard]] QModelIndex indexByChatId(int chat_id);
  static void setCurrentId(int current_id);

 Q_SIGNALS:
  void chatAdded(int id);
  void errorOccurred(const QString& error);
  void userCreated(const User& user, const QString& token);
  void newMessage(Message& message);
  void chatUpdated(int chat_id);

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
  void onAuthenticate(QNetworkReply* reply);
  std::optional<User> onGetUser(QNetworkReply* reply);
  QNetworkRequest getRequestWithToken(QUrl endpoint);

  QUrl url_;
  INetworkAccessManager* net_manager_;
  ICache* cache_;
  QWebSocket* socket_;
  QString current_token_;
  std::unique_ptr<ChatModel> chat_model_;
  std::unique_ptr<UserModel> user_model_;
  ChatMap chats_by_id_;
  MessageModelMap message_models_by_chat_id_;
};

#endif  // MODEL_H
