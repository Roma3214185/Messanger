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
#include "usecases/socketusecase.h"

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

  MessageModel* getMessageModel(long long chat_id);
  [[nodiscard]] std::optional<QString> checkToken();
  void deleteToken() const;
  void saveData(const QString& token, long long current_id);
  void logout();

  inline SessionUseCase* session() { return session_use_case_.get(); }
  inline MessageUseCase* message() { return message_use_case_.get(); }
  inline UserUseCase* user() { return user_use_case_.get(); }
  inline ChatUseCase* chat() { return chat_use_case_.get(); }
  inline DataManager* getDataManager() { return data_manager_; }
  inline TokenManager* getTokenManager() { return token_manager_.get(); }
  inline SocketUseCase* socket() { return socket_use_case_.get(); }

  void setupConnections();

 private:
  ICache* cache_;

  std::unique_ptr<ChatModel> chat_model_;
  std::unique_ptr<UserModel> user_model_;

  std::unique_ptr<SessionManager> session_manager_;
  std::unique_ptr<ChatManager>    chat_manager_;
  std::unique_ptr<MessageManager> message_manager_;
  std::unique_ptr<UserManager>    user_manager_;
  std::unique_ptr<SocketManager>  socket_manager_;
  std::unique_ptr<SocketUseCase> socket_use_case_;
  DataManager*    data_manager_;

  std::unique_ptr<TokenManager> token_manager_;
  std::unique_ptr<ChatUseCase> chat_use_case_;
  std::unique_ptr<UserUseCase> user_use_case_;
  std::unique_ptr<MessageUseCase> message_use_case_;
  std::unique_ptr<SessionUseCase> session_use_case_;

};

#endif  // MODEL_H
