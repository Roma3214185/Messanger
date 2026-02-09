#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <memory>
#include <optional>
#include <unordered_map>

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
class JsonService;
class UseCaseRepository;
class TokenManager;
class SessionUseCase;
class MessageUseCase;
class UserUseCase;
class ChatUseCase;
class SocketUseCase;

using ChatId = long long;
using ChatPtr = std::shared_ptr<ChatBase>;
using MessageModelPtr = std::shared_ptr<MessageModel>;

class ModelAttorney;

class Model : public QObject {
  Q_OBJECT
 public:
  Model(UseCaseRepository *use_case_repository, ICache *cache, TokenManager *token_manager, ISocket *socket,
        DataManager *data_manager);

  ChatModel *chatModel() const noexcept;
  UserModel *userModel() const noexcept;
  MessageModel *messageModel(long long chat_id);

  [[nodiscard]] std::optional<QString> checkToken();
  void saveData(const QString &token, long long current_id);
  void clearAll();
  void setupConnections();

  SessionUseCase *session() const;
  MessageUseCase *message() const;
  UserUseCase *user() const;
  ChatUseCase *chat() const;
  SocketUseCase *socket() const;
  DataManager *dataManager() const;
  TokenManager *tokenManager() const;

 private:
  TokenManager *token_manager_;
  std::unique_ptr<ChatModel> chat_model_;
  std::unique_ptr<UserModel> user_model_;
  UseCaseRepository *use_case_repository_;
  ICache *cache_;
  DataManager *data_manager_;
};

#endif  // MODEL_H
