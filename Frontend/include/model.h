#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QUrl>
#include <QWebSocket>
#include <memory>
#include <optional>
#include <unordered_map>

#include "managers/Managers.h"
#include "usecases/chatusecase.h"
#include "usecases/messageusecase.h"
#include "usecases/sessionusecase.h"
#include "usecases/userusecase.h"
#include "managers/TokenManager.h"

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
class DataManager;
class ISocket;
class ChatUseCase;
class MessageUseCase;
class UserUseCase;

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

  inline ChatModel* getChatModel() const { return chat_model_.get(); }
  inline UserModel* getUserModel() const { return user_model_.get(); }

  MessageModel*                 getMessageModel(long long chat_id);

  void checkToken();
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

  inline SessionUseCase& session() { return *session_use_case_; }
  inline MessageUseCase& message() { return *message_use_case_; }
  inline UserUseCase& user() { return *user_use_case_; }
  inline ChatUseCase& chat() { return *chat_use_case_; }

 Q_SIGNALS:
  void chatAdded(long long id);
  void errorOccurred(const QString& error);
  void userCreated(const User& user, const QString& token);
  void newResponce(QJsonObject& message);
  void chatUpdated(long long chat_id);

 private:
  void onMessageReceived(const QString& msg);
  void setupConnections();
  void addMessageWithUpdatingChatList(const Message& msg, const User& user, long long chat_id, MessageModelPtr message_model);

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

  std::unique_ptr<TokenManager> token_manager_;
  std::unique_ptr<ChatUseCase> chat_use_case_;
  std::unique_ptr<UserUseCase> user_use_case_;
  std::unique_ptr<MessageUseCase> message_use_case_;
  std::unique_ptr<SessionUseCase> session_use_case_;

};

#endif  // MODEL_H
