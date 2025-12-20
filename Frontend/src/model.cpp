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

namespace {

auto getRequestWithToken(QUrl endpoint, QString current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

template <typename T>
T waitForFuture(QFuture<T>& future) {
  QFutureWatcher<T> watcher;
  watcher.setFuture(future);

  QEventLoop loop;
  QObject::connect(&watcher, &QFutureWatcher<T>::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return future.result();
}

}  // namespace

Model::Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash, ISocket* socket, DataManager* data_manager)
    : cache_(cash),
      chat_model_(std::make_unique<ChatModel>()),
      user_model_(std::make_unique<UserModel>()),
      current_token_(""),
      session_manager_(std::make_unique<SessionManager>(netManager, url)),
      chat_manager_(std::make_unique<ChatManager>(netManager, url)),
      message_manager_(std::make_unique<MessageManager>(netManager, url)),
      user_manager_(std::make_unique<UserManager>(netManager, url)),
      socket_manager_(std::make_unique<SocketManager>(socket, url)),
      data_manager_(data_manager) {
  LOG_INFO("[Model::Model] Initialized Model with URL: '{}'", url.toString().toStdString());

  setupConnections();
}

void Model::setupConnections() {
  connect(chat_model_.get(), &ChatModel::chatUpdated, this, [this](int chatId) -> void {
    Q_EMIT chatUpdated(chatId);
  });

  connect(session_manager_.get(), &SessionManager::userCreated, this, &Model::userCreated);
  connect(socket_manager_.get(), &SocketManager::newTextFromSocket, this, &Model::onMessageReceived);
}

Model::~Model() {}

auto Model::indexByChatId(long long chat_id) -> QModelIndex {
  std::optional<int> idx = chat_model_->findIndexByChatId(chat_id);
  if (!idx.has_value()) {
    LOG_ERROR("Model::indexByChatId — chatId '{}' not found", chat_id);
    return {};
  }
  return chat_model_->index(*idx);
}

void Model::checkToken() {
  PROFILE_SCOPE("Model::checkToken");
  auto tokenOpt = cache_->get("TOKEN");
  if (tokenOpt) {
    LOG_INFO("[checkToken] Token found: '{}'", *tokenOpt);
    session_manager_->authenticateWithToken(QString::fromStdString(*tokenOpt));
  } else {
    LOG_WARN("[checkToken] No token found");
  }
}

void Model::saveToken(const QString& token) {
  current_token_ = token;
  cache_->saveToken("TOKEN", token.toStdString());
  LOG_INFO("[saveToken] Token saved");
}

void Model::deleteToken() const {
  cache_->deleteToken("TOKEN");
  LOG_INFO("[deleteToken] Token deleted");
}

void Model::setCurrentUserId(long long current_id) { MessageModel::setCurrentUserId(current_id); }

void Model::signIn(const LogInRequest& login_request) { session_manager_->signIn(login_request); }

void Model::signUp(const SignUpRequest& request) { session_manager_->signUp(request); }

ChatPtr Model::loadChat(long long chat_id) {
  auto future = chat_manager_->loadChat(current_token_, chat_id);
  return waitForFuture(future);
}

auto Model::getPrivateChatWithUser(long long user_id) -> ChatPtr {
  PROFILE_SCOPE("Model::getPrivateChatWithUser");
  auto chat_ptr = data_manager_->getPrivateChatWithUser(user_id);
  if (chat_ptr) return chat_ptr;

  LOG_INFO("Private chat for this user '{}' not found", user_id);
  auto chat = createPrivateChat(user_id);
  LOG_INFO("Private chat for this user '{}' is created, id '{}'", chat->chat_id, user_id);
  addChatInFront(chat);  // (!) emit chatAdded -> load chat history if exist
  return chat;
}

auto Model::createPrivateChat(long long user_id) -> ChatPtr {
  auto future = chat_manager_->createPrivateChat(current_token_, user_id);
  return waitForFuture(future);
}

auto Model::loadChats() -> QList<ChatPtr> {
  auto future = chat_manager_->loadChats(current_token_);
  return waitForFuture(future);
}

auto Model::getChatMessages(long long chat_id, int limit) -> QList<Message> {
  int before_id = 0;
  //TODO: cache request result for {chat_id before_id}

  auto message_model = data_manager_->getMessageModel(chat_id);
  if (message_model) {
    auto oldestMessage = message_model->getOldestMessage();
    if (oldestMessage) {
      LOG_INFO("Last message with id '{}' and text '{}'",
               oldestMessage->id,
               oldestMessage->text.toStdString());
      before_id = oldestMessage->id;
    }
  }

  LOG_INFO("[getChatMessages] Loading messages for chatId={}, beforeId = '{}'", chat_id, before_id);
  auto future = message_manager_->getChatMessages(current_token_, chat_id, before_id, limit);
  return waitForFuture(future);
}

MessageModel* Model::getMessageModel(long long chat_id) {
  PROFILE_SCOPE("Model::getMessageModel");
  auto message_model = data_manager_->getMessageModel(chat_id);
  if (!message_model) {
    LOG_ERROR("Message model is nullptr for id {}", chat_id);
    throw std::runtime_error("Nullptr messagemodel");
  }
  LOG_INFO("Message model is getted from Model");

  return message_model.get();
}

void Model::addMessageToChat(long long chat_id, const Message& msg) {
  PROFILE_SCOPE("Model::addMessageToChat");
  auto chat = data_manager_->getChat(chat_id);
  if (!chat) {
    LOG_INFO("Chat with id '{}' isn't exist", chat_id);
    auto chat = loadChat(msg.chatId);  // u can receive new message from group/user if u
                                       // delete for youtself and from newUser
    addChatInFront(chat);
  }

  auto message_model = data_manager_->getMessageModel(chat_id);
  auto user = data_manager_->getUser(msg.senderId);

  if (!user) {
    LOG_INFO("There is no info about user {} in cache", msg.senderId);
    auto user_from_server = getUser(msg.senderId);
    if(!user_from_server) {
      LOG_ERROR("Server can't find info about user {}", msg.senderId);
      return;
    }
    data_manager_->saveUser(*user_from_server);
    user = user_from_server;
  } else {
    getUserAsync(msg.senderId);
  }

  message_model->addMessage(msg, *user);
  auto last_chat_message = message_model->getLastMessage();
  chat_model_->updateChatInfo(chat_id, last_chat_message);
}

void Model::addOfflineMessageToChat(long long chat_id, User user, const Message& msg) {
  auto message_model = data_manager_->getMessageModel(chat_id);
  if(!message_model) {   // TODO: make one function add message(offline + online)
    LOG_ERROR("Invalid message_model");
    return;
  }
  message_model->addMessage(msg, user);
}

void Model::addChatInFront(const ChatPtr& chat) {
  PROFILE_SCOPE("Model::addChatInFront");
  addChat(chat);
  chat_model_->realocateChatInFront(chat->chat_id);
}

void Model::sendMessage(const Message& msg) {
  PROFILE_SCOPE("Model::sendMessage");

  if (msg.text.trimmed().isEmpty()) {
    LOG_WARN(
        "[sendMessage] Empty message skipped. chatId={}, senderId={}", msg.chatId, msg.senderId);
    return;
  }

  auto json = QJsonObject{{"type", "send_message"},
                          {"sender_id", msg.senderId},
                          {"chat_id", msg.chatId},
                          {"text", msg.text},
                          {"timestamp", msg.timestamp.toString()},
                          {"local_id", msg.local_id}};

  QString new_message =
      QString(QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));

  socket_manager_->sendText(new_message);
  LOG_INFO("[sendMessage] Sent message to chatId={} from user {}: '{}'",
           msg.chatId,
           msg.senderId,
           msg.text.toStdString());
}

auto Model::getUser(long long user_id) -> optional<User> {
  auto future = user_manager_->getUser(user_id, current_token_);
  return waitForFuture(future);
}

void Model::getUserAsync(long long user_id) {
  QFuture<std::optional<User>> future = user_manager_->getUser(user_id, current_token_);

  auto *watcher = new QFutureWatcher<std::optional<User>>(this);
  connect(watcher, &QFutureWatcherBase::finished, this, [this, watcher, user_id]() {
    auto userOpt = watcher->result();
    watcher->deleteLater();

    if (!userOpt) {
      LOG_ERROR("Can't get info about user {}", user_id);
      return;
    }

    data_manager_->saveUser(*userOpt);
  });

  watcher->setFuture(future);
}

auto Model::getNumberOfExistingChats() const -> int {
  int size = data_manager_->getNumberOfExistingChats();
  LOG_INFO("[getNumberOfExistingChats] Number of chats={}", size);
  return size;
}

void Model::logout() {
  PROFILE_SCOPE("Model::logout");
  LOG_INFO("[logout] Logging out user");

  socket_manager_->close();

  clearAllChats();
  clearAllMessages();
  deleteToken();
  current_token_.clear();
  chat_model_->clear();
  LOG_INFO("[logout] Logout complete");
}

void Model::clearAllChats() {
  data_manager_->clearAllChats();
  LOG_INFO("[clearAllChats] clearAllChats complete");
}

void Model::clearAllMessages() {
  data_manager_->clearAllMessageModels();
  LOG_INFO("[clearAllMessages] clearAllMessages complete");
}

auto Model::findUsers(const QString& text) -> QList<User> {
  auto future = user_manager_->findUsersByTag(text, current_token_);
  return waitForFuture(future);
}

void Model::addChat(const ChatPtr& chat) {
  PROFILE_SCOPE("Model::addChat");
  data_manager_->addChat(chat);
  chat_model_->addChat(chat);
  Q_EMIT chatAdded(chat->chat_id);
}

ChatPtr Model::getChat(long long chat_id) { return data_manager_->getChat(chat_id); }

void Model::initSocket(long long user_id) { socket_manager_->initSocket(user_id); }

void Model::onMessageReceived(const QString& msg) {
  PROFILE_SCOPE("Model::onMessageReceived");
  LOG_INFO("[onMessageReceived] Message received from user {}: ", msg.toStdString());

  QJsonParseError parseError;
  auto            doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    LOG_ERROR("[onMessageReceived] Failed JSON parse: '{}'",
              parseError.errorString().toStdString());
    Q_EMIT errorOccurred("Invalid JSON received: " + parseError.errorString());
    return;
  }

  auto json_responce = doc.object();
  Q_EMIT newResponce(json_responce);
}

void Model::connectSocket() { socket_manager_->connectSocket(); }

void Model::createChat(long long chat_id) {
  PROFILE_SCOPE("Model::createChat");
  auto chat = data_manager_->getChat(chat_id);
  if (chat) {
    LOG_INFO("[Chat '{}' already exist", chat_id);
    return;
  }
  auto new_chat = loadChat(chat_id);
  fillChatHistory(chat_id);
  chat_model_->addChatInFront(new_chat);
}

void Model::fillChatHistory(long long chat_id) {
  PROFILE_SCOPE("Model::fillChatHistory");
  auto message_history = getChatMessages(chat_id);
  LOG_INFO("[fillChatHistory] For chat '{}' loaded '{}' messages", chat_id, message_history.size());
  auto message_model = data_manager_->getMessageModel(chat_id);

  if (message_history.empty()) {
    return;
  }

  for (auto message : message_history) {
    auto user = getUser(message.senderId);
    if (!user) {
      LOG_ERROR("[fillChatHistory] getUser failed for message '{}'", message.id);
    } else {
      LOG_INFO("[fillChatHistory] For message '{}' user is '{}'", message.id, user->id);
    }

    message_model->addMessage(message, *user);
  }

  auto last_message = message_model->getLastMessage();
  chat_model_->updateChatInfo(chat_id, last_message);
}

auto Model::getChatModel() const -> ChatModel* { return chat_model_.get(); }

auto Model::getUserModel() const -> UserModel* { return user_model_.get(); }
