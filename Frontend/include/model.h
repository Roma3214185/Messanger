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

using ChatId          = int;
using ChatPtr         = std::shared_ptr<ChatBase>;
using MessageModelPtr = std::shared_ptr<MessageModel>;

class Model : public QObject {
  Q_OBJECT

 public:
  explicit Model(const QUrl&            url,
                 INetworkAccessManager* net_manager,
                 ICache*                cache,
                 ISocket*               socket);
  ~Model();

  ChatModel*                    getChatModel() const;
  UserModel*                    getUserModel() const;
  [[nodiscard]] MessageModelPtr createMessageModel(int chat_id);
  MessageModel*                 getMessageModel(int chat_id);

  void checkToken();
  void signIn(const LogInRequest& login_request);
  void signUp(const SignUpRequest& request);
  void initSocket(int user_id);
  void connectSocket();

  [[nodiscard]] ChatPtr     loadChat(int chat_id);
  [[nodiscard]] QList<User> findUsers(const QString& text);
  std::optional<User>       getUser(int user_id);
  ChatPtr                   createPrivateChat(int user_id);

  QList<Message> getChatMessages(int chat_id, int limit = 20);

  void                         sendMessage(const Message& msg);
  [[nodiscard]] QList<ChatPtr> loadChats();
  void                         authenticateWithToken(const QString& token);
  void                         fillChatHistory(int chat_id);
  void                         addChat(const ChatPtr& chat);
  void                         addChatInFront(const ChatPtr& chat);
  void                         createChat(int chat_id);
  void    addMessageToChat(int chat_id, const Message& msg, bool in_front = false);
  void    deleteToken() const;
  ChatPtr getPrivateChatWithUser(int user_id);
  void    saveToken(const QString& token);
  void    clearAllChats();
  void    clearAllMessages();
  void    logout();
  ChatPtr getChat(int chat_id);

  int         getNumberOfExistingChats() const;
  QModelIndex indexByChatId(int chat_id);
  static void setCurrentId(int current_id);

 Q_SIGNALS:
  void chatAdded(int id);
  void errorOccurred(const QString& error);
  void userCreated(const User& user, const QString& token);
  void newResponce(QJsonObject& message);
  void chatUpdated(int chat_id);

 private:
  void onMessageReceived(const QString& msg);
  void setupConnections();

  ICache* cache_;
  QString current_token_;

  std::unique_ptr<ChatModel> chat_model_;
  std::unique_ptr<UserModel> user_model_;

  SessionManager* session_manager_;
  ChatManager*    chat_manager_;
  MessageManager* message_manager_;
  UserManager*    user_manager_;
  SocketManager*  socket_manager_;
  DataManager*    data_manager_;
};

#endif  // MODEL_H
