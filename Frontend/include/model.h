#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QUrl>
#include <QWebSocket>
#include <memory>
#include <optional>
#include <unordered_map>

#include "managers/Managers.h"
#include "managers/TokenManager.h"
#include "usecases/chatusecase.h"
#include "usecases/messageusecase.h"
#include "usecases/sessionusecase.h"
#include "usecases/socketusecase.h"
#include "usecases/userusecase.h"

struct ChatBase;
class MessageModel;
class ChatModel;
class UserModel;
struct Message;
class QNetworkReply;
class INetworkAccessManager;
class ICache;
struct SignUpRequest;
struct LogInRequest;
struct User;
class MessageInfo;
class DataManager;
class ISocket;
class ChatUseCase;
class MessageUseCase;
class UserUseCase;

using ChatId          = long long;
using ChatPtr         = std::shared_ptr<ChatBase>;
using MessageModelPtr = std::shared_ptr<MessageModel>;

class ModelAttorney;

class Model : public QObject {
  Q_OBJECT

 public:
  explicit Model(const QUrl&            url,
                 INetworkAccessManager* net_manager,
                 ICache*                cache,
                 ISocket*               socket,
                 DataManager*           data_manager);

  ChatModel* getChatModel() const noexcept { return chat_model_.get(); }
  UserModel* getUserModel() const noexcept { return user_model_.get(); }

  MessageModel*                        getMessageModel(long long chat_id);
  [[nodiscard]] std::optional<QString> checkToken();
  void                                 deleteToken() const;
  void                                 saveData(const QString& token, long long current_id);
  void                                 logout();
  void setupConnections();

  SessionUseCase* session() const noexcept { return session_use_case_.get(); }
  MessageUseCase* message() const noexcept { return message_use_case_.get(); }
  UserUseCase* user() const noexcept { return user_use_case_.get(); }
  ChatUseCase* chat() const noexcept { return chat_use_case_.get(); }
  DataManager* dataManager() const noexcept { return data_manager_; }
  TokenManager* tokenManager() const noexcept { return token_manager_.get(); }
  SocketUseCase* socket() const noexcept { return socket_use_case_.get(); }

 private:
  ICache* cache_;

  std::unique_ptr<ChatModel> chat_model_;
  std::unique_ptr<UserModel> user_model_;

  DataManager*                  data_manager_;
  std::unique_ptr<TokenManager> token_manager_;

  std::unique_ptr<SocketUseCase>  socket_use_case_;
  std::unique_ptr<ChatUseCase>    chat_use_case_;
  std::unique_ptr<UserUseCase>    user_use_case_;
  std::unique_ptr<MessageUseCase> message_use_case_;
  std::unique_ptr<SessionUseCase> session_use_case_;

  friend class ModelAttorney;
};

// class ModelAttorney {
//   private:
//     static SessionUseCase* session(Model& model) const noexcept { return model.session_use_case_.get(); }
//     static MessageUseCase* message(Model& model) const noexcept { return model.message_use_case_.get(); }
//     static UserUseCase* user(Model& model) const noexcept { return model.user_use_case_.get(); }
//     static ChatUseCase* chat(Model& model) const noexcept { return model.chat_use_case_.get(); }
//     static DataManager* dataManager(Model& model) const noexcept { return model.data_manager_; }
//     static TokenManager* tokenManager(Model& model) const noexcept { return model.token_manager_.get(); }
//     static SocketUseCase* socket(Model& model) const noexcept { return model.socket_use_case_.get(); }
//     friend class Presenter;
// };

#endif  // MODEL_H
