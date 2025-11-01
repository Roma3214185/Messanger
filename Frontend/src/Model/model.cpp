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
#include "Managers/SessionManager/sessionmanager.h"
#include "Managers/ChatManager/chatmanager.h"
#include "Managers/MessageManager/messagemanager.h"
#include "Managers/UserManager/usermanager.h"
#include "Managers/SocketManager/socketmanager.h"

namespace {

auto getRequestWithToken(QUrl endpoint, QString current_token) -> QNetworkRequest {
  auto request = QNetworkRequest(endpoint);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization", current_token.toUtf8());
  return request;
}

}  // namespace

Model::Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash,
             QWebSocket* socket)
    : cache_(cash)
    , chat_model_(std::make_unique<ChatModel>())
    , user_model_(std::make_unique<UserModel>())
    , current_token_("")
    , chats_by_id_()
    , message_models_by_chat_id_()
    , session_manager_(new SessionManager(netManager, url))
    , chat_manager_(new ChatManager(netManager, url))
    , message_manager_(new MessageManager(netManager, url))
    , user_manager_(new UserManager(netManager, url))
    , socket_manager_(new SocketManager(socket, url))
{

  LOG_INFO("[Model::Model] Initialized Model with URL: '{}'",
          url.toString().toStdString());

  setupConnections();
}

void Model::setupConnections(){
  connect(chat_model_.get(), &ChatModel::chatUpdated, this,
          [this](int chatId) -> void { Q_EMIT chatUpdated(chatId); });

  connect(session_manager_, &SessionManager::userCreated, this, &Model::userCreated);
  connect(socket_manager_, &SocketManager::newTextFromSocket, this, &Model::onMessageReceived);
}

Model::~Model() {
  delete session_manager_;
  delete chat_manager_;
  delete message_manager_;
  delete user_manager_;
  delete socket_manager_;
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

void Model::setCurrentId(int current_id) {
  MessageModel::setCurrentUserId(current_id);
}

void Model::signIn(const LogInRequest& login_request) {
  session_manager_->signIn(login_request);
}

void Model::signUp(const SignUpRequest& request) {
  session_manager_->signUp(request);
}

ChatPtr Model::loadChat(int chat_id) {
  return chat_manager_->loadChat(current_token_, chat_id);
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
  return chat_manager_->createPrivateChat(current_token_, user_id);
}

auto Model::loadChats() -> QList<ChatPtr> {
  return chat_manager_->loadChats(current_token_);
}

auto Model::getChatMessages(int chat_id, int limit) -> QList<Message> {
  int before_id = 0;
  if (message_models_by_chat_id_.count(chat_id)) {
    auto firstMessage = message_models_by_chat_id_[chat_id]->getFirstMessage();
    if (firstMessage) {
      LOG_INFO("Last message with id '{}' and text '{}'", firstMessage->id,
               firstMessage->text.toStdString());
      before_id = firstMessage->id;
    }
  }
  LOG_INFO("[getChatMessages] Loading messages for chatId={}, beforeId = '{}'",
           chat_id, before_id);

  return message_manager_->getChatMessages(current_token_, chat_id, before_id, limit);
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

  QString new_message = QString(
      QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));

  socket_manager_->sendText(new_message);
  LOG_INFO("[sendMessage] Sent message to chatId={} from user {}: '{}'",
          msg.chatId, msg.senderId, msg.text.toStdString());
}

auto Model::getUser(int user_id) -> optional<User> {
  return user_manager_->getUser(user_id);  //TODO(roma): send token to verify user blocks u
}

auto Model::getNumberOfExistingChats() const -> int {
  LOG_INFO("[getNumberOfExistingChats] Number of chats={}", chats_by_id_.size());
  return chats_by_id_.size();
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
  chats_by_id_.clear();
  LOG_INFO("[clearAllChats] clearAllChats complete");
}

void Model::clearAllMessages() {
  message_models_by_chat_id_.clear();
  LOG_INFO("[clearAllMessages] clearAllMessages complete");
}

auto Model::findUsers(const QString& text) -> QList<User> {
  return user_manager_->findUsersByTag(text);
}

void Model::addChat(const ChatPtr& chat) {
  PROFILE_SCOPE("Model::addChat");
  chats_by_id_[chat->chat_id] = chat;
  chat_model_->addChat(chat);
  Q_EMIT chatAdded(chat->chat_id);
}

ChatPtr Model::getChat(int chat_id){
  auto iter = chats_by_id_.find(chat_id);
  if(iter == chats_by_id_.end()) return nullptr;
  return iter->second;
}

void Model::connectSocket(int user_id) {
  socket_manager_->connectSocket(user_id);
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

  for (auto message : messageHistory) {
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
