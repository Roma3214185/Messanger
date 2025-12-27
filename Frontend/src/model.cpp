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
#include "managers/chatmanager.h"
#include "managers/datamanager.h"
#include "managers/messagemanager.h"
#include "managers/sessionmanager.h"
#include "managers/socketmanager.h"
#include "managers/usermanager.h"
#include "models/UserModel.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"

Model::Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash, ISocket* socket, DataManager* data_manager)
    : cache_(cash)
    , chat_model_(std::make_unique<ChatModel>())
    , user_model_(std::make_unique<UserModel>())
    , session_manager_(std::make_unique<SessionManager>(netManager, url))
    , chat_manager_(std::make_unique<ChatManager>(netManager, url))
    , message_manager_(std::make_unique<MessageManager>(netManager, url))
    , user_manager_(std::make_unique<UserManager>(netManager, url))
    , socket_manager_(std::make_unique<SocketManager>(socket, url))
    , data_manager_(data_manager)
    , token_manager_(std::make_unique<TokenManager>())
    , socket_use_case_(std::make_unique<SocketUseCase>(socket_manager_.get()))
    , chat_use_case_(std::make_unique<ChatUseCase>(chat_manager_.get(), data_manager_, chat_model_.get(), token_manager_.get()))
    , user_use_case_(std::make_unique<UserUseCase>(data_manager_, user_manager_.get(), token_manager_.get()))
    , message_use_case_(std::make_unique<MessageUseCase>(data_manager_, message_manager_.get(), token_manager_.get()))
    , session_use_case_(std::make_unique<SessionUseCase>(session_manager_.get())) {
  LOG_INFO("[Model::Model] Initialized Model with URL: '{}'", url.toString().toStdString());
}

void Model::setupConnections() {
  connect(getDataManager(), &DataManager::chatAdded, this,  [this] (const ChatPtr& added_chat) {
    DBC_REQUIRE(added_chat != nullptr);
    message()->getChatMessagesAsync(added_chat->chat_id);
    getChatModel()->addChat(added_chat);
  });

  connect(getDataManager(), &DataManager::messageAdded, this, [this](const Message& message){
    user_use_case_->getUserAsync(message.sender_id);
    auto last_message = getMessageModel(message.chat_id)->getLastMessage();
    chat_model_->updateChatInfo(message.chat_id, last_message);
    //  todo: getChatAsync() and there check: if exists, skip
    //manager_->message()->getChatMessagesAsync(message.chatId);
  });

  connect(getDataManager(), &DataManager::chatAdded, this, [this](const ChatPtr& chat){
    DBC_REQUIRE(chat != nullptr); //todo: in chat class make isValid fucntion that check all self field
    message_use_case_->getChatMessagesAsync(chat->chat_id);
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

void Model::saveData(const QString& token, long long current_id) {
  DBC_REQUIRE(!token.isEmpty() && current_id > 0);
  token_manager_->setData(token, current_id);
  cache_->saveToken("TOKEN", token.toStdString());
  LOG_INFO("[saveToken] Token saved");
}

MessageModel* Model::getMessageModel(long long chat_id) {
  DBC_REQUIRE(chat_id > 0);
  auto message_model = data_manager_->getMessageModel(chat_id);
  DBC_REQUIRE(message_model);
  return message_model.get();
}

void Model::logout() {
  PROFILE_SCOPE();
  LOG_INFO("[logout] Logging out user");

  socket_manager_->close();
  data_manager_->clearAll();
  cache_->deleteToken("TOKEN");
  token_manager_->resetData();
  chat_use_case_->clearAllChats(); // todo: delete each chat, and non signal delete delete clear models of th
  chat_model_->clear();
  token_manager_->resetData();
  LOG_INFO("[logout] Logout complete");
}
