#include "Model/model.h"

#include <memory>
#include <optional>

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QString>
#include <QUrlQuery>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebSockets/QWebSocket>

#include "ChatModel/chatmodel.h"
#include "headers/INetworkAccessManager.h"
#include "headers/ICache.h"
#include "headers/SignUpRequest.h"
#include "headers/JsonService.h"
#include "headers/User.h"
#include "Debug_profiling.h"
#include "MessageModel/messagemodel.h"
#include "UserModel/UserModel.h"

Model::Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash,
             QWebSocket* socket)
    : url_(url),
      net_manager_(netManager),
      cache_(cash),
      socket_(socket),
      chat_model_(std::make_unique<ChatModel>()),
      user_model_(std::make_unique<UserModel>()),
      current_token_(""),
      chats_by_id_(),
      message_models_by_chat_id_() {

  LOG_INFO("[Model::Model] Initialized Model with URL: '{}'",
          url.toString().toStdString());

  connect(chat_model_.get(), &ChatModel::chatUpdated, this,
          [this](int chatId) -> void { Q_EMIT chatUpdated(chatId); });
}

auto Model::indexByChatId(int chat_id) -> QModelIndex {
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
    authenticateWithToken(QString::fromStdString(*tokenOpt));
  } else {
    LOG_WARN("[checkToken] No token found");
  }
}

void Model::saveToken(const QString& token) const {
  cache_->saveToken("TOKEN", token.toStdString());
  LOG_INFO("[saveToken] Token saved");
}

void Model::deleteToken() const {
  cache_->deleteToken("TOKEN");
  LOG_INFO("[deleteToken] Token deleted");
}

void Model::setCurrentId(int current_id) {
  MessageModel::setCurrentUserId(current_id);
}

void Model::signIn(const LogInRequest& login_request) {
  PROFILE_SCOPE("Model::signIn");
  LOG_INFO("[signIn] Attempting login for email '{}'",
          login_request.email.toStdString());

  QUrl endpoint = url_.resolved(QUrl("/auth/login"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject body{{"email", login_request.email},
                   {"password", login_request.password}};
  auto reply = net_manager_->post(req, QJsonDocument(body).toJson());

  connect(reply, &QNetworkReply::finished, this,
          [this, reply]() -> void { onSignInFinished(reply); });
}

void Model::onSignInFinished(QNetworkReply* reply) {
  PROFILE_SCOPE("Model::onSignInFinished");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onSignInFinished] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return;
  }

  auto jsonResponse = QJsonDocument::fromJson(reply->readAll());
  auto responseObj = jsonResponse.object();
  auto createdUser =
      JsonService::getUserFromResponse(responseObj["user"].toObject());
  current_token_ = responseObj["token"].toString();

  LOG_INFO("[onSignInFinished] Login success. User: '{}', Token: '{}'",
           createdUser.name.toStdString(), current_token_.toStdString());
  Q_EMIT userCreated(createdUser, current_token_);
}

void Model::signUp(const SignUpRequest& request) {
  PROFILE_SCOPE("Model::signUp");
  LOG_INFO("[signUp] Registering new user: '{}'", request.email.toStdString());

  QUrl endpoint = url_.resolved(QUrl("/auth/register"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject body{{"email", request.email},
                   {"password", request.password},
                   {"name", request.name},
                   {"tag", request.tag}};
  auto reply = net_manager_->post(req, QJsonDocument(body).toJson());

  connect(reply, &QNetworkReply::finished, this,
          [this, reply]() -> void { onSignUpFinished(reply); });
}

void Model::onSignUpFinished(QNetworkReply* reply) {
  PROFILE_SCOPE("Model::onSignUpFinished");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onSignUpFinished] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return;
  }

  auto jsonResponse = QJsonDocument::fromJson(reply->readAll());
  auto responseObj = jsonResponse.object();
  auto createdUser =
      JsonService::getUserFromResponse(responseObj["user"].toObject());
  current_token_ = responseObj["token"].toString();

  LOG_INFO("[onSignUpFinished] Registration success. User: '{}', Token: '{}'",
           createdUser.name.toStdString(), current_token_.toStdString());
  Q_EMIT userCreated(createdUser, current_token_);
}

ChatPtr Model::getChat(int chat_id) {
  auto iter = chats_by_id_.find(chat_id);
  if (iter == chats_by_id_.end()) return nullptr;
  return iter->second;
}

void Model::connectSocket(int user_id) {
  PROFILE_SCOPE("Model::connectSocket");
  connect(socket_, &QWebSocket::connected,
          [this, user_id]() -> void { onSocketConnected(user_id); });
  connect(socket_, &QWebSocket::textMessageReceived, this,
          &Model::onMessageReceived);
  socket_->open(QUrl("ws://localhost:8086/ws"));
  LOG_INFO("[connectSocket] Connecting WebSocket for userId={}", user_id);
}

void Model::onSocketConnected(int user_id) {
  PROFILE_SCOPE("Model::onSocketConnected");
  QJsonObject json{{"type", "init"}, {"userId", user_id}};
  const QString msg =
      QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
  socket_->sendTextMessage(msg);
  LOG_INFO("[onSocketConnected] WebSocket initialized for userId={}", user_id);
}

void Model::onMessageReceived(const QString& msg) {
  PROFILE_SCOPE("Model::onMessageReceived");
  QJsonParseError parseError;
  auto doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    LOG_ERROR("[onMessageReceived] Failed JSON parse: '{}'",
              parseError.errorString().toStdString());
    Q_EMIT errorOccurred("Invalid JSON received: " + parseError.errorString());
    return;
  }

  auto new_msg = JsonService::getMessageFromJson(doc.object());
  LOG_INFO("[onMessageReceived] Message received from user {}: '{}'",
           new_msg.senderId, new_msg.text.toStdString());
  Q_EMIT newMessage(new_msg);
}

auto Model::loadChat(int chatId) -> ChatPtr {
  PROFILE_SCOPE("Model::loadChat");
  LOG_INFO("[loadChat] Loading chat id={}", chatId);

  QUrl url("http://localhost:8081");
  QUrl endpoint = url.resolved(QUrl(QString("/chats/%1").arg(chatId)));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setRawHeader("Authorization", current_token_.toUtf8());

  auto* reply = net_manager_->get(req);
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  auto chat = onChatLoaded(reply);
  if (chat) {
    LOG_INFO("[loadChat] Chat loaded id={}", chatId);
  } else {
    LOG_ERROR("[loadChat] Failed to load chat id={}", chatId);
  }
  return chat;
}

auto Model::onChatLoaded(QNetworkReply* reply) -> ChatPtr {
  PROFILE_SCOPE("Model::onChatLoaded");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onChatLoaded] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return nullptr;
  }

  auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isObject()) {
    LOG_ERROR("[onChatLoaded] Invalid JSON, expected object at root");
    Q_EMIT errorOccurred("loadChat: invalid JSON root");
    return nullptr;
  }

  auto chat = JsonService::getChatFromJson(doc.object());
  return chat;
}

auto Model::getPrivateChatWithUser(int user_id) -> ChatPtr {
  PROFILE_SCOPE("Model::getPrivateChatWithUser");
  for (auto [_, chat] : chats_by_id_) {
    if (chat->isPrivate()) {
      auto* pchat = static_cast<PrivateChat*>(chat.get());
      if (pchat->user_id == user_id) {
        LOG_INFO("Found private chat for this user '{}' and id '{}'",
                 pchat->title.toStdString(), pchat->chat_id);
        return chat;
      }
    }
  }
  LOG_INFO("Private chat for this user '{}' not found", user_id);
  auto chat = createPrivateChat(user_id);
  LOG_INFO("Private chat for this user '{}' is created, id '{}'", chat->chat_id);
  addChatInFront(chat);  // (!) emit chatAdded -> load chat history if exist
  return chat;
}

auto Model::createPrivateChat(int user_id) -> ChatPtr {
  PROFILE_SCOPE("Model::createPrivateChat");
  QUrl url("http://localhost:8081");
  auto endpoint = url.resolved(QUrl("/chats/private"));
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token_.toUtf8());
  auto body = QJsonObject{
      {"user_id", user_id},
  };
  auto reply = net_manager_->post(request, QJsonDocument(body).toJson());
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();
  auto newChat = onCreatePrivateChat(reply);
  return newChat;
}

auto Model::onCreatePrivateChat(QNetworkReply* reply) -> ChatPtr {
  PROFILE_SCOPE("Model::onCreatePrivateChat");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onCreatePrivateChat] error '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred("onCreatePrivateChat" + reply->errorString());
    return nullptr;
  }
  auto responseData = reply->readAll();
  auto doc = QJsonDocument::fromJson(responseData);
  if (!doc.isObject()) {
    LOG_ERROR("[onCreatePrivateChat] Invalid JSON: expected object at root");
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return nullptr;
  }
  auto responseObj = doc.object();
  if (responseObj["chat_type"].toString() != "PRIVATE") {
    LOG_ERROR("Error in model create private chat returned group chat");
    Q_EMIT errorOccurred(
        "Error in model create private chat returned group chat");
    return nullptr;
  }
  auto new_chat = JsonService::getPrivateChatFromJson(responseObj);
  LOG_INFO("Private chat created with id '{}' ", new_chat->chat_id);
  return new_chat;
}

void Model::onAuthenticate(QNetworkReply* reply) {
  PROFILE_SCOPE("Model::onAuthenticate");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    spdlog::warn("OnAuthenticate failed: '{}'", reply->errorString().toStdString());
    return;
  }

  auto response_data = reply->readAll();
  QJsonParseError parse_error;
  auto json_response = QJsonDocument::fromJson(response_data, &parse_error);
  if (parse_error.error != QJsonParseError::NoError ||
      !json_response.isObject()) {
    LOG_ERROR("Sign me failed: invalid JSON - '{}'",
              parse_error.errorString().toStdString());
    return;
  }

  auto response_obj = json_response.object();

  if (!response_obj.contains("user") || !response_obj["user"].isObject()) {
    LOG_ERROR("Sign me failed: JSON does not contain 'user' object");
    return;
  }

  auto created_user =
      JsonService::getUserFromResponse(response_obj["user"].toObject());

  if (!response_obj.contains("token") || !response_obj["token"].isString()) {
    spdlog::warn("Sign me succeeded but no token returned for user '{}'",
                 created_user.name.toStdString());
    current_token_.clear();
  } else {
    current_token_ = response_obj["token"].toString();
    LOG_INFO("Sign me success: user '{}' with token '{}'",
             created_user.name.toStdString(), current_token_.toStdString());
  }

  Q_EMIT userCreated(created_user, current_token_);
}

auto Model::loadChats() -> QList<ChatPtr> {
  PROFILE_SCOPE("Model::loadChats");
  LOG_INFO("[loadChats] Loading all chats");

  QUrl url("http://localhost:8081");
  QUrl endpoint = url.resolved(QUrl("/chats"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setRawHeader("Authorization", current_token_.toUtf8());

  auto* reply = net_manager_->get(req);
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return onLoadChats(reply);
}

auto Model::onLoadChats(QNetworkReply* reply) -> QList<ChatPtr> {
  PROFILE_SCOPE("Model::onLoadChats");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onLoadChats] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred(reply->errorString());
    return {};
  }

  QByteArray raw = reply->readAll();
  spdlog::warn("[onLoadChats] RAW RESPONSE ({} bytes):\n{}", raw.size(),
               raw.toStdString());

  auto doc = QJsonDocument::fromJson(raw);
  if (!doc.isObject() || !doc.object().contains("chats") ||
      !doc.object()["chats"].isArray()) {
    LOG_ERROR("LoadChats: invalid JSON");
    Q_EMIT errorOccurred("LoadChats: invalid JSON");
    return {};
  }

  auto chats = QList<ChatPtr>{};
  for (const auto& val : doc.object()["chats"].toArray()) {
    auto chat = JsonService::getChatFromJson(val.toObject());
    if (chat)
      chats.append(chat);
    else
      spdlog::warn("[onLoadChats] Skipping invalid chat object");
  }

  LOG_INFO("[onLoadChats] Loaded {} chats", chats.size());
  return chats;
}

auto Model::getChatMessages(int chatId) -> QList<Message> {
  constexpr int kMessageToLoad = 20;
  return getChatMessages(chatId, kMessageToLoad);
}

auto Model::getChatMessages(int chatId, int limit) -> QList<Message> {
  PROFILE_SCOPE("Model::getChatMessages");
  int beforeId = 0;
  if (message_models_by_chat_id_.count(chatId)) {
    auto firstMessage = message_models_by_chat_id_[chatId]->getFirstMessage();
    if (firstMessage) {
      LOG_INFO("Last message with id '{}' and text '{}'", firstMessage->id,
               firstMessage->text.toStdString());
      beforeId = firstMessage->id;
    }
  }
  LOG_INFO("[getChatMessages] Loading messages for chatId={}, beforeId = '{}'",
           chatId, beforeId);

  QUrl url("http://localhost:8082");
  QUrl endpoint = url.resolved(QUrl(QString("/messages/%1").arg(chatId)));
  LOG_INFO("For chatId '{}' limit is '{}' and beforeId '{}'", chatId, limit,
           beforeId);
  QUrlQuery query;
  query.addQueryItem("limit", QString::number(limit));
  query.addQueryItem("beforeId", QString::number(beforeId));
  endpoint.setQuery(query);
  auto request = getRequestWithToken(endpoint);
  auto* reply = net_manager_->get(request);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return onGetChatMessages(reply);
}

auto Model::getRequestWithToken(QUrl endpoint) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token_.toUtf8());
  return request;
}

auto Model::onGetChatMessages(QNetworkReply* reply) -> QList<Message> {
  PROFILE_SCOPE("Model::onGetChatMessages");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onGetChatMessages] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred("[network] " + reply->errorString());
    return {};
  }

  auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isArray()) {
    LOG_ERROR("[onGetChatMessages] Invalid JSON: expected array");
    Q_EMIT errorOccurred("Invalid JSON: expected array at root");
    return {};
  }

  QList<Message> messages;
  for (const auto& val : doc.array()) {
    messages.append(JsonService::getMessageFromJson(val.toObject()));
  }
  LOG_INFO("[onGetChatMessages] Loaded {} messages", messages.size());
  return messages;
}

MessageModel* Model::getMessageModel(int chat_id) {
  PROFILE_SCOPE("Model::getMessageModel");
  auto chat_iter = message_models_by_chat_id_.find(chat_id);
  if (chat_iter == message_models_by_chat_id_.end()) {
    LOG_INFO("Chat with id '{}' isn't exist", chat_id);
    createMessageModel(chat_id);
  }
  return message_models_by_chat_id_[chat_id].get();
}

MessageModelPtr Model::createMessageModel(int chat_id) {
  PROFILE_SCOPE("Model::createMessageModel");
  auto msgModel = std::make_shared<MessageModel>();
  message_models_by_chat_id_[chat_id] = msgModel;
  return msgModel;
}

void Model::addMessageToChat(int chat_id, const Message& msg, bool in_front) {
  PROFILE_SCOPE("Model::addMessageToChat");
  auto chatIter = chats_by_id_.find(chat_id);
  if (chatIter == chats_by_id_.end()) {
    LOG_INFO("Chat with id '{}' isn't exist", chat_id);
    auto chat =
        loadChat(msg.chatId);  // u can receive new message from group/user if u
                               // delete for youtself and from newUser
    addChatInFront(chat);
  }

  auto messageModel = message_models_by_chat_id_[chat_id];
  auto user = getUser(msg.senderId);

  if (!user) {
    LOG_ERROR("Use with id '{}' isn't exist", msg.senderId);
    Q_EMIT errorOccurred(QString("Server doesn't return info about user id(") +
                         QString::fromStdString(std::to_string(msg.senderId)));
    return;
  }

  if (in_front) {
    messageModel->addMessage(msg, *user, true);
    //chat_model_->updateChat(chat_id, msg.text, msg.timestamp);
  } else {
    messageModel->addMessage(msg, *user, false);
    chat_model_->updateChat(chat_id, msg.text, msg.timestamp);
  }
}

void Model::addChatInFront(const ChatPtr& chat) {
  PROFILE_SCOPE("Model::addChatInFront");
  addChat(chat);
  chat_model_->realocateChatInFront(chat->chat_id);
}

void Model::sendMessage(const Message& msg) {
  PROFILE_SCOPE("Model::sendMessage");

  if (msg.text.trimmed().isEmpty()) {
    LOG_WARN("[sendMessage] Empty message skipped. chatId={}, senderId={}",
                 msg.chatId, msg.senderId);
    return;
  }

  auto json = QJsonObject{
      {"type", "send_message"},
      {"sender_id", msg.senderId},
      {"chat_id", msg.chatId},
      {"text", msg.text},
      {"timestamp", msg.timestamp.toString()},
      {"local_id", msg.local_id}
  };

  socket_->sendTextMessage(
      QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
  LOG_INFO("[sendMessage] Sent message to chatId={} from user {}: '{}'",
          msg.chatId, msg.senderId, msg.text.toStdString());
}

auto Model::getUser(int userId) -> optional<User> {
  PROFILE_SCOPE("Model::getUser");
  LOG_INFO("[getUser] Loading user id={}", userId);

  QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(userId)));
  QNetworkRequest req(endpoint);
  auto* reply = net_manager_->get(req);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return onGetUser(reply);
}

auto Model::onGetUser(QNetworkReply* reply) -> optional<User> {
  PROFILE_SCOPE("Model::onGetUser");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>
      guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    LOG_ERROR("[onGetUser] Network error: '{}'",
              reply->errorString().toStdString());
    Q_EMIT errorOccurred("get user: " + reply->errorString());
    return std::nullopt;
  }

  auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isObject()) {
    LOG_ERROR("[onGetUser] Invalid JSON: expected object at root");
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return std::nullopt;
  }

  auto user = JsonService::getUserFromResponse(doc.object());
  LOG_INFO("[onGetUser] User loaded: '{}'", user.name.toStdString());
  return user;
}

auto Model::getNumberOfExistingChats() const -> int {
  LOG_INFO("[getNumberOfExistingChats] Number of chats={}", chats_by_id_.size());
  return chats_by_id_.size();
}

void Model::logout() {
  PROFILE_SCOPE("Model::logout");
  LOG_INFO("[logout] Logging out user");

  if (socket_) {
    socket_->disconnect();
    socket_->close();
  }

  clearAllChats();
  clearAllMessages();
  deleteToken();
  current_token_.clear();
  chat_model_->clear();
  LOG_INFO("[logout] Logout complete");
}

void Model::clearAllChats() {
  chats_by_id_.clear();
  LOG_INFO("[clearAllChats] clearAllChats complete");
}

void Model::clearAllMessages() {
  message_models_by_chat_id_.clear();
  LOG_INFO("[clearAllMessages] clearAllMessages complete");
}

auto Model::findUsers(const QString& text) -> QList<User> {
  QUrl endpoint =
      url_.resolved(QUrl(QString("/users/search?tag=%1").arg(text)));
  auto request = QNetworkRequest(endpoint);
  auto* reply = net_manager_->get(request);
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();
  auto users = onFindUsers(reply);
  return users;
}

void Model::authenticateWithToken(const QString& token) {
  QUrl url("http://localhost:8083");
  QUrl endpoint = url.resolved(QUrl("/auth/me"));
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", token.toUtf8());
  auto* reply = net_manager_->get(request);
  QObject::connect(reply, &QNetworkReply::finished, this,
                   [this, reply]() -> void { onAuthenticate(reply); });
}

auto Model::onFindUsers(QNetworkReply* reply) -> QList<User> {
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
  if (reply->error() != QNetworkReply::NoError) {
    Q_EMIT errorOccurred("onFindUsers" + reply->errorString());
    return {};
  }
  auto responseData = reply->readAll();
  auto doc = QJsonDocument::fromJson(responseData);
  if (!doc.isObject()) {
    Q_EMIT errorOccurred("Invalid JSON: expected object at root");
    return {};
  }
  auto rootObj = doc.object();
  auto arr = rootObj["users"].toArray();
  QList<User> users;
  for (const auto& value : std::as_const(arr)) {
    auto obj = value.toObject();
    auto user = JsonService::getUserFromResponse(obj);
    users.append(user);
  }
  return users;
}

void Model::addChat(const ChatPtr& chat) {
  PROFILE_SCOPE("Model::addChat");
  chats_by_id_[chat->chat_id] = chat;
  chat_model_->addChat(chat);
  Q_EMIT chatAdded(chat->chat_id);
}

void Model::createChat(int chat_id) {
  PROFILE_SCOPE("Model::createChat");
  auto chatIterator = chats_by_id_.find(chat_id);
  if (chatIterator != chats_by_id_.end()) {
    LOG_INFO("[Chat '{}' already exist", chat_id);
    return;
  }
  auto chat = loadChat(chat_id);
  fillChatHistory(chat_id);
  chat_model_->addChatInFront(chat);
}

void Model::fillChatHistory(int chat_id) {
  PROFILE_SCOPE("Model::fillChatHistory");
  auto messageHistory = getChatMessages(chat_id);
  LOG_INFO("[fillChatHistory] For chat '{}' loaded '{}' messages", chat_id,
           messageHistory.size());
  auto messageModel = std::make_shared<MessageModel>(this);
  message_models_by_chat_id_[chat_id] = messageModel;

  if (messageHistory.empty()) {
    return;
  }

  chat_model_->updateChat(chat_id, messageHistory.front().text,
                        messageHistory.front().timestamp);

  for (auto message : std::as_const(messageHistory)) {
    auto user = getUser(message.senderId);
    if (!user) {
      LOG_ERROR("[fillChatHistory] getUser failed for message '{}'",
                message.id);
    } else {
      LOG_INFO("[fillChatHistory] For message '{}' user is '{}'", message.id,
               user->id);
    }

    messageModel->addMessage(message, *user, true);
  }
}

auto Model::getChatModel() const -> ChatModel* { return chat_model_.get(); }

auto Model::getUserModel() const -> UserModel* { return user_model_.get(); }
