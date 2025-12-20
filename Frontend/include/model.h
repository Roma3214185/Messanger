#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QUrl>
#include <QWebSocket>
#include <memory>
#include <optional>
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
class SessionManager;
class ChatManager;
class MessageManager;
class UserManager;
class SocketManager;
class DataManager;
class ISocket;

using ChatId          = long long;
using ChatPtr         = std::shared_ptr<ChatBase>;
using MessageModelPtr = std::shared_ptr<MessageModel>;

class Model : public QObject {
  Q_OBJECT

 public:
  explicit Model(const QUrl&            url,
                 INetworkAccessManager* net_manager,
                 ICache*                cache,
                 ISocket*               socket,
                 DataManager* data_manager);
  ~Model();

  ChatModel*                    getChatModel() const;
  UserModel*                    getUserModel() const;
  MessageModel*                 getMessageModel(long long chat_id);

  void checkToken();
  void signIn(const LogInRequest& login_request);
  void signUp(const SignUpRequest& request);
  void initSocket(long long user_id);
  void connectSocket();

  [[nodiscard]] ChatPtr     loadChat(long long chat_id);
  [[nodiscard]] QList<User> findUsers(const QString& text);
  std::optional<User>       getUser(long long user_id);
  ChatPtr                   createPrivateChat(long long user_id);

  QList<Message> getChatMessages(long long chat_id, int limit = 20);

  void                         sendMessage(const Message& msg);
  [[nodiscard]] QList<ChatPtr> loadChats();
  void                         authenticateWithToken(const QString& token);
  void                         getUserAsync(long long user_id);
  void                         fillChatHistory(long long chat_id);
  void                         addChat(const ChatPtr& chat);
  void                         addChatInFront(const ChatPtr& chat);
  void                         createChat(long long chat_id);
  void    addMessageToChat(long long chat_id, const Message& msg);
  void    addOfflineMessageToChat(long long chat_id, User, const Message&);
  void    deleteToken() const;
  ChatPtr getPrivateChatWithUser(long long user_id);
  void    saveToken(const QString& token);
  void    clearAllChats();
  void    clearAllMessages();
  void    logout();
  ChatPtr getChat(long long chat_id);

  int         getNumberOfExistingChats() const;
  QModelIndex indexByChatId(long long chat_id);
  static void setCurrentUserId(long long current_id);

 Q_SIGNALS:
  void chatAdded(long long id);
  void errorOccurred(const QString& error);
  void userCreated(const User& user, const QString& token);
  void newResponce(QJsonObject& message);
  void chatUpdated(long long chat_id);

 private:
  void onMessageReceived(const QString& msg);
  void setupConnections();

  ICache* cache_;
  QString current_token_;

  std::unique_ptr<ChatModel> chat_model_;
  std::unique_ptr<UserModel> user_model_;

  std::unique_ptr<SessionManager> session_manager_;
  std::unique_ptr<ChatManager>    chat_manager_;
  std::unique_ptr<MessageManager> message_manager_;
  std::unique_ptr<UserManager>    user_manager_;
  std::unique_ptr<SocketManager>  socket_manager_;
  DataManager*    data_manager_;
};

#endif  // MODEL_H
