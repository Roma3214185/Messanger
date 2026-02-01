#include "model.h"

#include <QEventLoop>
#include <QFuture>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QString>
#include <QUrlQuery>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebSockets/QWebSocket>
#include <memory>
#include <optional>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "dto/SignUpRequest.h"
#include "dto/User.h"
#include "interfaces/ICache.h"
#include "interfaces/INetworkAccessManager.h"
#include "interfaces/ISocket.h"
#include "managers/TokenManager.h"
#include "managers/chatmanager.h"
#include "managers/datamanager.h"
#include "managers/messagemanager.h"
#include "managers/sessionmanager.h"
#include "managers/socketmanager.h"
#include "managers/usermanager.h"
#include "models/UserModel.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"
#include "usecases/IUseCaseRepository.h"

Model::Model(IUseCaseRepository *use_case_repostirory, ICache *cash, TokenManager *token_manager, ISocket *socket,
             DataManager *data_manager)
    : use_case_repository_(use_case_repostirory),
      cache_(cash),
      token_manager_(token_manager),
      // entity_factory_(std::make_unique<JsonService>(token_manager_.get())),
      chat_model_(std::make_unique<ChatModel>()),
      user_model_(std::make_unique<UserModel>()),
      data_manager_(data_manager) {}

void Model::setupConnections() {
  connect(data_manager_, &DataManager::chatAdded, this, [this](const ChatPtr &added_chat) {
    DBC_REQUIRE(added_chat != nullptr);
    message()->getChatMessagesAsync(added_chat->chat_id);
    getChatModel()->addChat(added_chat);
  });

  connect(data_manager_, &DataManager::messageAdded, this, [this](const Message &message) {
    user()->getUserAsync(message.sender_id);
    auto last_message = getMessageModel(message.chat_id)->getLastMessage();
    chat_model_->updateChatInfo(message.chat_id, last_message);
    //  todo: getChatAsync() and there check: if exists, skip
    // manager_->message()->getChatMessagesAsync(message.chatId);
  });

  connect(data_manager_, &DataManager::chatAdded, this, [this](const ChatPtr &chat) {
    DBC_REQUIRE(chat != nullptr);  // todo: in chat class make isValid
                                   // fucntion that check all self field
    message()->getChatMessagesAsync(chat->chat_id);
  });

  connect(data_manager_, &DataManager::messageDeleted, this, [this](const Message &deleted_message) {
    DBC_REQUIRE(deleted_message.checkInvariants());
    auto message_model = getMessageModel(deleted_message.chat_id);
    message_model->deleteMessage(deleted_message);
  });
}

std::optional<QString> Model::checkToken() {
  auto tokenOpt = cache_->get("TOKEN");
  if (tokenOpt) {
    LOG_INFO("[checkToken] Token found: '{}'", *tokenOpt);
    return QString::fromStdString(*tokenOpt);
  }

  LOG_WARN("[checkToken] No token found");
  return std::nullopt;
}

void Model::saveData(const QString &token, long long current_id) {
  DBC_REQUIRE(!token.isEmpty() && current_id > 0);
  token_manager_->setData(token, current_id);
  cache_->saveToken("TOKEN", token.toStdString());
  LOG_INFO("[saveToken] Token saved");
}

MessageModel *Model::getMessageModel(long long chat_id) {
  DBC_REQUIRE(chat_id > 0);
  auto message_model = data_manager_->getMessageModel(chat_id);
  DBC_REQUIRE(message_model);
  return message_model.get();
}

void Model::logout() {
  PROFILE_SCOPE();
  LOG_INFO("[logout] Logging out user");

  socket()->close();
  data_manager_->clearAll();
  cache_->deleteToken("TOKEN");
  token_manager_->resetData();
  chat_model_->clear();
  token_manager_->resetData();
  LOG_INFO("[logout] Logout complete");
}

ISessionUseCase *Model::session() const { return use_case_repository_->session(); }
IMessageUseCase *Model::message() const { return use_case_repository_->message(); }
IUserUseCase *Model::user() const { return use_case_repository_->user(); }
IChatUseCase *Model::chat() const { return use_case_repository_->chat(); }
DataManager *Model::dataManager() const { return data_manager_; }
TokenManager *Model::tokenManager() const { return token_manager_; }
ISocketUseCase *Model::socket() const { return use_case_repository_->socket(); }
// JsonService *Model::entities() const { return entity_factory_.get(); }

ChatModel *Model::getChatModel() const noexcept { return chat_model_.get(); }
UserModel *Model::getUserModel() const noexcept { return user_model_.get(); }
