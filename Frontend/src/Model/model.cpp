#include "model.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUrlQuery>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebSockets/QWebSocket>
#include <memory>
#include <optional>

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
      netManager(netManager),
      cash(cash),
      socket(socket),
      chatModel(std::make_unique<ChatModel>()),
      userModel(std::make_unique<UserModel>()),
      currentToken(""),
      chatsById(),
      messageModelsByChatId() {

  LOG_INFO("[Model::Model] Initialized Model with URL: '{}'",
          url.toString().toStdString());

  connect(chatModel.get(), &ChatModel::chatUpdated, this,
          [this](int chatId) -> void { Q_EMIT chatUpdated(chatId); });
}

auto Model::indexByChatId(int chat_id) -> QModelIndex {
  std::optional<int> idx = chatModel->findIndexByChatId(chat_id);
  if (!idx.has_value()) {
    LOG_ERROR("Model::indexByChatId — chatId '{}' not found", chat_id);
    return {};
  }
  return chatModel->index(*idx);
}

void Model::checkToken() {
  PROFILE_SCOPE("Model::checkToken");
  auto tokenOpt = cash->get("TOKEN");
  if (tokenOpt) {
    LOG_INFO("[checkToken] Token found: '{}'", *tokenOpt);
    signMe(QString::fromStdString(*tokenOpt));
  } else {
    LOG_WARN("[checkToken] No token found");
  }
}

void Model::saveToken(const QString& token) const {
  cash->saveToken("TOKEN", token.toStdString());
  LOG_INFO("[saveToken] Token saved");
}

void Model::deleteToken() const {
  cash->deleteToken("TOKEN");
  LOG_INFO("[deleteToken] Token deleted");
}

void Model::setCurrentId(int current_id) { MessageModel::setCurrentUserId(current_id); }

void Model::signIn(const LogInRequest& login_request) {
  PROFILE_SCOPE("Model::signIn");
  LOG_INFO("[signIn] Attempting login for email '{}'", login_request.email.toStdString());

  QUrl endpoint = url_.resolved(QUrl("/auth/login"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject body{{"email", login_request.email}, {"password", login_request.password}};
  auto reply = netManager->post(req, QJsonDocument(body).toJson());

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
  currentToken = responseObj["token"].toString();

  LOG_INFO("[onSignInFinished] Login success. User: '{}', Token: '{}'",
           createdUser.name.toStdString(), currentToken.toStdString());
  Q_EMIT userCreated(createdUser, currentToken);
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
  auto reply = netManager->post(req, QJsonDocument(body).toJson());

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
  currentToken = responseObj["token"].toString();

  LOG_INFO("[onSignUpFinished] Registration success. User: '{}', Token: '{}'",
           createdUser.name.toStdString(), currentToken.toStdString());
  Q_EMIT userCreated(createdUser, currentToken);
}

void Model::connectSocket(int user_id) {
  PROFILE_SCOPE("Model::connectSocket");
  connect(socket, &QWebSocket::connected, [this, user_id]() -> void { onSocketConnected(user_id); });
  connect(socket, &QWebSocket::textMessageReceived, this,
          &Model::onMessageReceived);
  socket->open(QUrl("ws://localhost:8086/ws"));
  LOG_INFO("[connectSocket] Connecting WebSocket for userId={}", user_id);
}

void Model::onSocketConnected(int id) {
  PROFILE_SCOPE("Model::onSocketConnected");
  QJsonObject json{{"type", "init"}, {"userId", id}};
  QString msg =
      QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
  socket->sendTextMessage(msg);
  LOG_INFO("[onSocketConnected] WebSocket initialized for userId={}", id);
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

  auto newMsg = JsonService::getMessageFromJson(doc.object());
  LOG_INFO("[onMessageReceived] Message received from user {}: '{}'",
           newMsg.senderId, newMsg.text.toStdString());
  Q_EMIT newMessage(newMsg);
}

auto Model::loadChat(int chatId) -> ChatPtr {
  PROFILE_SCOPE("Model::loadChat");
  LOG_INFO("[loadChat] Loading chat id={}", chatId);

  QUrl url("http://localhost:8081");
  QUrl endpoint = url.resolved(QUrl(QString("/chats/%1").arg(chatId)));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setRawHeader("Authorization", currentToken.toUtf8());

  auto* reply = netManager->get(req);
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  auto chat = onChatLoaded(reply);
  if (chat) {
    LOG_INFO("[loadChat] Chat loaded id={}", chatId);
  }else {
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

auto Model::getPrivateChatWithUser(int userId) -> ChatPtr {
  PROFILE_SCOPE("Model::getPrivateChatWithUser");
  for (auto [chatId, chat] : chatsById) {
    if (chat->isPrivate()) {
      auto* pchat = static_cast<PrivateChat*>(chat.get());
      if (pchat->userId == userId) {
        LOG_INFO("Found private chat for this user '{}' and id '{}'",
                 pchat->title.toStdString(), pchat->chatId);
        return chat;
      }
    }
  }
  LOG_INFO("Private chat for this user '{}' not found", userId);
  auto chat = createPrivateChat(userId);
  LOG_INFO("Private chat for this user '{}' is created, id '{}'", chat->chatId);
  addChatInFront(chat);  // (!) emit chatAdded -> load chat history if exist
  return chat;
}

auto Model::createPrivateChat(int userId) -> ChatPtr {
  PROFILE_SCOPE("Model::createPrivateChat");
  QUrl url("http://localhost:8081");
  auto endpoint = url.resolved(QUrl("/chats/private"));
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", currentToken.toUtf8());
  auto body = QJsonObject{
      {"user_id", userId},
  };
  auto reply = netManager->post(request, QJsonDocument(body).toJson());
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
  auto newChat = JsonService::getPrivateChatFromJson(responseObj);
  LOG_INFO("Private chat created with id '{}' ", newChat->chatId);
  return newChat;
}

void Model::onSignMe(QNetworkReply* reply) {
  PROFILE_SCOPE("Model::onSignMe");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

  if (reply->error() != QNetworkReply::NoError) {
    spdlog::warn("Sign me failed: '{}'", reply->errorString().toStdString());
    return;
  }

  auto responseData = reply->readAll();
  QJsonParseError parseError;
  auto jsonResponse = QJsonDocument::fromJson(responseData, &parseError);
  if (parseError.error != QJsonParseError::NoError ||
      !jsonResponse.isObject()) {
    LOG_ERROR("Sign me failed: invalid JSON - '{}'",
              parseError.errorString().toStdString());
    return;
  }

  auto responseObj = jsonResponse.object();

  if (!responseObj.contains("user") || !responseObj["user"].isObject()) {
    LOG_ERROR("Sign me failed: JSON does not contain 'user' object");
    return;
  }

  auto createdUser =
      JsonService::getUserFromResponse(responseObj["user"].toObject());

  if (!responseObj.contains("token") || !responseObj["token"].isString()) {
    spdlog::warn("Sign me succeeded but no token returned for user '{}'",
                 createdUser.name.toStdString());
    currentToken.clear();
  } else {
    currentToken = responseObj["token"].toString();
    LOG_INFO("Sign me success: user '{}' with token '{}'",
             createdUser.name.toStdString(), currentToken.toStdString());
  }

  Q_EMIT userCreated(createdUser, currentToken);
}

auto Model::loadChats() -> QList<ChatPtr> {
  PROFILE_SCOPE("Model::loadChats");
  LOG_INFO("[loadChats] Loading all chats");

  QUrl url("http://localhost:8081");
  QUrl endpoint = url.resolved(QUrl("/chats"));
  QNetworkRequest req(endpoint);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setRawHeader("Authorization", currentToken.toUtf8());

  auto* reply = netManager->get(req);
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
  if (messageModelsByChatId.count(chatId)) {
    auto firstMessage = messageModelsByChatId[chatId]->getFirstMessage();
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
  auto* reply = netManager->get(request);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return onGetChatMessages(reply);
}

auto Model::getRequestWithToken(QUrl endpoint) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", currentToken.toUtf8());
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

auto Model::getMessageModel(int chatId) -> MessageModel* {
  PROFILE_SCOPE("Model::getMessageModel");
  auto chatIter = messageModelsByChatId.find(chatId);
  if (chatIter == messageModelsByChatId.end()) {
    LOG_INFO("Chat with id '{}' isn't exist", chatId);
    createMessageModel(chatId);
  }
  return messageModelsByChatId[chatId].get();
}

auto Model::createMessageModel(int chatId) -> MessageModelPtr {
  PROFILE_SCOPE("Model::createMessageModel");
  auto msgModel = std::make_shared<MessageModel>();
  messageModelsByChatId[chatId] = msgModel;
  return msgModel;
}

void Model::addMessageToChat(int chatId, const Message& msg){
  addMessageToChat(chatId, msg, true);
}

void Model::addMessageToChat(int chatId, const Message& msg, bool infront) {
  PROFILE_SCOPE("Model::addMessageToChat");
  auto chatIter = chatsById.find(chatId);
  if (chatIter == chatsById.end()) {
    LOG_INFO("Chat with id '{}' isn't exist", chatId);
    auto chat =
        loadChat(msg.chatId);  // u can receive new message from group/user if u
                               // delete for youtself and from newUser
    addChatInFront(chat);
  }

  auto messageModel = messageModelsByChatId[chatId];
  auto user = getUser(msg.senderId);

  if (!user) {
    LOG_ERROR("Use with id '{}' isn't exist", msg.senderId);
    Q_EMIT errorOccurred(QString("Server doesn't return info about user id(") +
                         QString::fromStdString(std::to_string(msg.senderId)));
    return;
  }

  if (infront) {
    messageModel->addMessage(msg, *user);
    chatModel->updateChat(chatId, msg.text, msg.timestamp);
    // chatModel->realocateChatInFront(chatId);
  } else {
    messageModel->addMessageInBack(msg, *user);
  }
}

void Model::addChatInFront(const ChatPtr& chat) {
  PROFILE_SCOPE("Model::addChatInFront");
  addChat(chat);
  chatModel->realocateChatInFront(chat->chatId);
}

void Model::sendMessage(const MessageInfo& msg) {
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
      {"timestamp", QDateTime::currentDateTime().toString()},
  };

  socket->sendTextMessage(
      QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
  LOG_INFO("[sendMessage] Sent message to chatId={} from user {}: '{}'", msg.chatId,
           msg.senderId, msg.text.toStdString());
}

auto Model::getUser(int userId) -> optional<User> {
  PROFILE_SCOPE("Model::getUser");
  LOG_INFO("[getUser] Loading user id={}", userId);

  QUrl endpoint = url_.resolved(QUrl(QString("/users/%1").arg(userId)));
  QNetworkRequest req(endpoint);
  auto* reply = netManager->get(req);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return onGetUser(reply);
}

auto Model::onGetUser(QNetworkReply* reply) -> optional<User> {
  PROFILE_SCOPE("Model::onGetUser");
  QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

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
  LOG_INFO("[getNumberOfExistingChats] Number of chats={}", chatsById.size());
  return chatsById.size();
}

void Model::logout() {
  PROFILE_SCOPE("Model::logout");
  LOG_INFO("[logout] Logging out user");

  if (socket) {
    socket->disconnect();
    socket->close();
  }

  clearAllChats();
  clearAllMessages();
  deleteToken();
  currentToken.clear();
  chatModel->clear();
  LOG_INFO("[logout] Logout complete");
}

void Model::clearAllChats() {
  chatsById.clear();
  LOG_INFO("[clearAllChats] clearAllChats complete");
}

void Model::clearAllMessages() {
  messageModelsByChatId.clear();
  LOG_INFO("[clearAllMessages] clearAllMessages complete");
}

auto Model::findUsers(const QString& text) -> QList<User> {
  QUrl endpoint =
      url_.resolved(QUrl(QString("/users/search?tag=%1").arg(text)));
  auto request = QNetworkRequest(endpoint);
  auto* reply = netManager->get(request);
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();
  auto users = onFindUsers(reply);
  return users;
}

void Model::signMe(const QString& token) {
  QUrl url("http://localhost:8083");
  QUrl endpoint = url.resolved(QUrl("/auth/me"));
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", token.toUtf8());
  auto* reply = netManager->get(request);
  QObject::connect(reply, &QNetworkReply::finished, this,
                   [this, reply]() -> void { onSignMe(reply); });
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
  for (const auto& value : arr) {
    auto obj = value.toObject();
    auto user = JsonService::getUserFromResponse(obj);
    users.append(user);
  }
  return users;
}

void Model::addChat(const ChatPtr& chat) {
  PROFILE_SCOPE("Model::addChat");
  chatsById[chat->chatId] = chat;
  chatModel->addChat(chat);
  Q_EMIT chatAdded(chat->chatId);
}

void Model::createChat(int chatId) {
  PROFILE_SCOPE("Model::createChat");
  auto chatIterator = chatsById.find(chatId);
  if (chatIterator != chatsById.end()) {
    LOG_INFO("[Chat '{}' already exist", chatId);
    return;
  }
  auto chat = loadChat(chatId);
  fillChatHistory(chatId);
  chatModel->addChatInFront(chat);
}

void Model::fillChatHistory(int chatId) {
  PROFILE_SCOPE("Model::fillChatHistory");
  auto messageHistory = getChatMessages(chatId);
  LOG_INFO("[fillChatHistory] For chat '{}' loaded '{}' messages", chatId,
           messageHistory.size());
  auto messageModel = std::make_shared<MessageModel>(this);
  messageModelsByChatId[chatId] = messageModel;

  if (messageHistory.empty()) {
    return;
  }

  chatModel->updateChat(chatId, messageHistory.front().text,
                        messageHistory.front().timestamp);

  for (auto message : messageHistory) {
    auto user = getUser(message.senderId);
    if (!user) {
      LOG_ERROR("[fillChatHistory] getUser failed for message '{}'",
                message.id);
    }else {
      LOG_INFO("[fillChatHistory] For message '{}' user is '{}'", message.id,
               user->id);
    }

    messageModel->addMessageInBack(message, *user);
  }
}

auto Model::getChatModel() -> ChatModel* { return chatModel.get(); }

auto Model::getUserModel() -> UserModel* { return userModel.get(); }
