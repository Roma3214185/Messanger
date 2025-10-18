#include "model.h"

#include <QtNetwork/QNetworkRequest>
#include <QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QEventLoop>
#include <QJsonArray>
#include "headers/JsonService.h"
#include "ChatModel/chatmodel.h"
#include "MessageModel/messagemodel.h"
#include "UserModel/UserModel.h"
#include "../../DebugProfiling/Debug_profiling.h"
#include <QUrlQuery>


Model::Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash, QWebSocket* socket)
    : url_(url)
    , netManager(netManager)
    , cash(cash)
    , socket(socket)
    , chatModel(std::make_unique<ChatModel>())
    , userModel(std::make_unique<UserModel>())
{
    LOG_INFO("[Model::Model] Initialized Model with URL: '{}'", url.toString().toStdString());

    connect(chatModel.get(), &ChatModel::chatUpdated, this, [this](int chatId){
        Q_EMIT chatUpdated(chatId);
    });
}

QModelIndex Model::indexByChatId(int chatId) const {
    std::optional<int> idx = chatModel->findIndexByChatId(chatId);
    if (!idx.has_value()) {
        LOG_ERROR("Model::indexByChatId — chatId '{}' not found", chatId);
        return QModelIndex();
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
        spdlog::warn("[checkToken] No token found");
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

void Model::signIn(const QString& email, const QString& password) {
    PROFILE_SCOPE("Model::signIn");
    LOG_INFO("[signIn] Attempting login for email '{}'", email.toStdString());

    QUrl endpoint = url_.resolved(QUrl("/auth/login"));
    QNetworkRequest req(endpoint);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body{{"email", email}, {"password", password}};
    auto reply = netManager->post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSignInFinished(reply);
    });
}

void Model::onSignInFinished(QNetworkReply* reply) {
    PROFILE_SCOPE("Model::onSignInFinished");
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("[onSignInFinished] Network error: '{}'", reply->errorString().toStdString());
        Q_EMIT errorOccurred(reply->errorString());
        return;
    }

    auto jsonResponse = QJsonDocument::fromJson(reply->readAll());
    auto responseObj = jsonResponse.object();
    auto createdUser = JsonService::getUserFromResponse(responseObj["user"].toObject());
    currentToken = responseObj["token"].toString();

    LOG_INFO("[onSignInFinished] Login success. User: '{}', Token: '{}'", createdUser.name.toStdString(), currentToken.toStdString());
    Q_EMIT userCreated(createdUser, currentToken);
}

void Model::signUp(const SignUpRequest& request) {
    PROFILE_SCOPE("Model::signUp");
    LOG_INFO("[signUp] Registering new user: '{}'", request.email.toStdString());

    QUrl endpoint = url_.resolved(QUrl("/auth/register"));
    QNetworkRequest req(endpoint);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body{
        {"email", request.email},
        {"password", request.password},
        {"name", request.name},
        {"tag", request.tag}
    };
    auto reply = netManager->post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSignUpFinished(reply);
    });
}

void Model::onSignUpFinished(QNetworkReply* reply) {
    PROFILE_SCOPE("Model::onSignUpFinished");
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("[onSignUpFinished] Network error: '{}'", reply->errorString().toStdString());
        Q_EMIT errorOccurred(reply->errorString());
        return;
    }

    auto jsonResponse = QJsonDocument::fromJson(reply->readAll());
    auto responseObj = jsonResponse.object();
    auto createdUser = JsonService::getUserFromResponse(responseObj["user"].toObject());
    currentToken = responseObj["token"].toString();

    LOG_INFO("[onSignUpFinished] Registration success. User: '{}', Token: '{}'", createdUser.name.toStdString(), currentToken.toStdString());
    Q_EMIT userCreated(createdUser, currentToken);
}

void Model::connectSocket(int id) {
    PROFILE_SCOPE("Model::connectSocket");
    connect(socket, &QWebSocket::connected, [=]() { onSocketConnected(id); });
    connect(socket, &QWebSocket::textMessageReceived, this, &Model::onMessageReceived);
    socket->open(QUrl("ws://localhost:8082/ws"));
    LOG_INFO("[connectSocket] Connecting WebSocket for userId={}", id);
}

void Model::onSocketConnected(int id) {
    PROFILE_SCOPE("Model::onSocketConnected");
    QJsonObject json{{"type", "init"}, {"userId", id}};
    QString msg = QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
    socket->sendTextMessage(msg);
    LOG_INFO("[onSocketConnected] WebSocket initialized for userId={}", id);
}

void Model::onMessageReceived(const QString& msg) {
    PROFILE_SCOPE("Model::onMessageReceived");
    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        LOG_ERROR("[onMessageReceived] Failed JSON parse: '{}'", parseError.errorString().toStdString());
        Q_EMIT errorOccurred("Invalid JSON received: " + parseError.errorString());
        return;
    }

    auto newMsg = JsonService::getMessageFromJson(doc.object());
    LOG_INFO("[onMessageReceived] Message received from user {}: '{}'", newMsg.senderId, newMsg.text.toStdString());
    Q_EMIT newMessage(newMsg);
}

ChatPtr Model::loadChat(int chatId) {
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
    if(chat) LOG_INFO("[loadChat] Chat loaded id={}", chatId);
    else LOG_ERROR("[loadChat] Failed to load chat id={}", chatId);
    return chat;
}

ChatPtr Model::onChatLoaded(QNetworkReply* reply) {
    PROFILE_SCOPE("Model::onChatLoaded");
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if(reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("[onChatLoaded] Network error: '{}'", reply->errorString().toStdString());
        Q_EMIT errorOccurred(reply->errorString());
        return nullptr;
    }

    auto doc = QJsonDocument::fromJson(reply->readAll());
    if(!doc.isObject()) {
        LOG_ERROR("[onChatLoaded] Invalid JSON, expected object at root");
        Q_EMIT errorOccurred("loadChat: invalid JSON root");
        return nullptr;
    }

    auto chat = JsonService::getChatFromJson(doc.object());
    return chat;
}

ChatPtr Model::getPrivateChatWithUser(int userId){
    PROFILE_SCOPE("Model::getPrivateChatWithUser");
    for (auto [chatId, chat] : chatsById) {
        if (chat->isPrivate()) {
            auto* pchat = static_cast<PrivateChat*>(chat.get());
            if (pchat->userId == userId) {
                LOG_INFO("Found private chat for this user '{}' and id '{}'", pchat->title.toStdString(), pchat->chatId);
                return chat;
            }
        }
    }
    LOG_INFO("Private chat for this user '{}' not found", userId);
    auto chat = createPrivateChat(userId);
    LOG_INFO("Private chat for this user '{}' is created, id '{}'", chat->chatId);
    addChatInFront(chat); // (!) emit chatAdded -> load chat history if exist
    return chat;
}

ChatPtr Model::createPrivateChat(int userId){
    PROFILE_SCOPE("Model::createPrivateChat");
    QUrl url("http://localhost:8081");
    auto endpoint = url.resolved(QUrl("/chats/private"));
    auto request = QNetworkRequest(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", currentToken.toUtf8());
    auto body = QJsonObject{ {"user_id", userId}, };
    auto reply = netManager->post(request, QJsonDocument(body).toJson());
    QEventLoop loop; QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); auto newChat = onCreatePrivateChat(reply);
    return newChat;
}

ChatPtr Model::onCreatePrivateChat(QNetworkReply* reply){
    PROFILE_SCOPE("Model::onCreatePrivateChat");
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
    if (reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("[onCreatePrivateChat] error '{}'", reply->errorString().toStdString());
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
    if(responseObj["chat_type"].toString() != "PRIVATE"){
        LOG_ERROR("Error in model create private chat returned group chat");
        Q_EMIT errorOccurred("Error in model create private chat returned group chat");
        return nullptr;
    }
    auto newChat = JsonService::getPrivateChatFromJson(responseObj);
    LOG_INFO("Private chat created with id '{}' ",newChat->chatId);
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
    if (parseError.error != QJsonParseError::NoError || !jsonResponse.isObject()) {
        LOG_ERROR("Sign me failed: invalid JSON - '{}'", parseError.errorString().toStdString());
        return;
    }

    auto responseObj = jsonResponse.object();

    if (!responseObj.contains("user") || !responseObj["user"].isObject()) {
        LOG_ERROR("Sign me failed: JSON does not contain 'user' object");
        return;
    }

    auto createdUser = JsonService::getUserFromResponse(responseObj["user"].toObject());

    if (!responseObj.contains("token") || !responseObj["token"].isString()) {
        spdlog::warn("Sign me succeeded but no token returned for user '{}'", createdUser.name.toStdString());
        currentToken.clear();
    } else {
        currentToken = responseObj["token"].toString();
        LOG_INFO("Sign me success: user '{}' with token '{}'", createdUser.name.toStdString(), currentToken.toStdString());
    }

    Q_EMIT userCreated(createdUser, currentToken);
}

QList<ChatPtr> Model::loadChats() {
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

QList<ChatPtr> Model::onLoadChats(QNetworkReply* reply) {
    PROFILE_SCOPE("Model::onLoadChats");
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if(reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("[onLoadChats] Network error: '{}'", reply->errorString().toStdString());
        Q_EMIT errorOccurred(reply->errorString());
        return {};
    }

    auto doc = QJsonDocument::fromJson(reply->readAll());
    if(!doc.isObject() || !doc.object().contains("chats") || !doc.object()["chats"].isArray()) {
        LOG_ERROR("[onLoadChats] Invalid JSON: missing 'chats' array");
        Q_EMIT errorOccurred("LoadChats: invalid JSON");
        return {};
    }

    QList<ChatPtr> chats;
    for(const auto& val : doc.object()["chats"].toArray()) {
        auto chat = JsonService::getChatFromJson(val.toObject());
        if(chat) chats.append(chat);
        else spdlog::warn("[onLoadChats] Skipping invalid chat object");
    }

    LOG_INFO("[onLoadChats] Loaded {} chats", chats.size());
    return chats;
}

QList<Message> Model::getChatMessages(int chatId, int limit) {
    PROFILE_SCOPE("Model::getChatMessages");
    int beforeId = 0;
    if(messageModelsByChatId.count(chatId)){
        auto firstMessage = messageModelsByChatId[chatId]->getFirstMessage();
        if(firstMessage) {
            LOG_INFO("Last message with id '{}' and text '{}'", firstMessage->id, firstMessage->text.toStdString());
            beforeId = firstMessage->id;
        }
    }
    LOG_INFO("[getChatMessages] Loading messages for chatId={}, beforeId = '{}'", chatId, beforeId);

    QUrl url("http://localhost:8082");
    QUrl endpoint = url.resolved(QUrl(QString("/messages/%1").arg(chatId)));
    LOG_INFO("For chatId '{}' limit is '{}' and beforeId '{}'", chatId, limit, beforeId);
    QUrlQuery query;
    query.addQueryItem("limit", QString::number(limit));
    query.addQueryItem("beforeId", QString::number(beforeId));
    endpoint.setQuery(query);

    QNetworkRequest req(endpoint);
    auto* reply = netManager->get(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return onGetChatMessages(reply);
}

QList<Message> Model::onGetChatMessages(QNetworkReply* reply) {
    PROFILE_SCOPE("Model::onGetChatMessages");
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if(reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("[onGetChatMessages] Network error: '{}'", reply->errorString().toStdString());
        Q_EMIT errorOccurred("[network] " + reply->errorString());
        return {};
    }

    auto doc = QJsonDocument::fromJson(reply->readAll());
    if(!doc.isArray()) {
        LOG_ERROR("[onGetChatMessages] Invalid JSON: expected array");
        Q_EMIT errorOccurred("Invalid JSON: expected array at root");
        return {};
    }

    QList<Message> messages;
    for(const auto& val : doc.array()) messages.append(JsonService::getMessageFromJson(val.toObject()));
    LOG_INFO("[onGetChatMessages] Loaded {} messages", messages.size());
    return messages;
}

MessageModel* Model::getMessageModel(int chatId){
    PROFILE_SCOPE("Model::getMessageModel");
    auto it = messageModelsByChatId.find(chatId);

    if(it == messageModelsByChatId.end()) {
        LOG_INFO("Chat with id '{}' isn't exist", chatId);
        createMessageModel(chatId);
    }
    return messageModelsByChatId[chatId].get();
}

MessageModelPtr Model::createMessageModel(int chatId){
    PROFILE_SCOPE("Model::createMessageModel");
    auto msgModel = std::make_shared<MessageModel>();
    messageModelsByChatId[chatId] = msgModel;
    return msgModel;
}

void Model::addMessageToChat(int chatId, const Message& msg, bool infront){
    PROFILE_SCOPE("Model::addMessageToChat");
    auto it = chatsById.find(chatId);
    if(it == chatsById.end()) {
        LOG_INFO("Chat with id '{}' isn't exist", chatId);
        auto chat = loadChat(msg.chatId); // u can receive new message from group/user if u delete for youtself and from newUser
        addChatInFront(chat);
    }

    auto messageModel = messageModelsByChatId[chatId];
    auto user = getUser(msg.senderId);

    if(!user) {
        LOG_ERROR("Use with id '{}' isn't exist", msg.senderId);
        Q_EMIT errorOccurred("Server doesn't return info about user id(" + msg.senderId);
        return;
    }

    if(infront) {
        messageModel->addMessage(msg, *user);
        chatModel->updateChat(chatId, msg.text, msg.timestamp);
        //chatModel->realocateChatInFront(chatId);
    } else messageModel->addMessageInBack(msg, *user);
}

void Model::addChatInFront(const ChatPtr& chat){
    PROFILE_SCOPE("Model::addChatInFront");
    addChat(chat);
    chatModel->realocateChatInFront(chat->chatId);
}

void Model::sendMessage(int chatId, int senderId, const QString& textToSend) {
    PROFILE_SCOPE("Model::sendMessage");

    if(textToSend.trimmed().isEmpty()) {
        spdlog::warn("[sendMessage] Empty message skipped. chatId={}, senderId={}", chatId, senderId);
        return;
    }

    auto json = QJsonObject{
                            {"type", "send_message"},
                            {"sender_id", senderId},
                            {"chat_id", chatId},
                            {"text", textToSend},
                            {"timestamp", QDateTime::currentDateTime().toString()},
                            };

    socket->sendTextMessage(QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
    LOG_INFO("[sendMessage] Sent message to chatId={} from user {}: '{}'", chatId, senderId, textToSend.toStdString());
}

optional<User> Model::getUser(int userId) {
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

optional<User> Model::onGetUser(QNetworkReply* reply) {
    PROFILE_SCOPE("Model::onGetUser");
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if(reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("[onGetUser] Network error: '{}'", reply->errorString().toStdString());
        Q_EMIT errorOccurred("get user: " + reply->errorString());
        return std::nullopt;
    }

    auto doc = QJsonDocument::fromJson(reply->readAll());
    if(!doc.isObject()) {
        LOG_ERROR("[onGetUser] Invalid JSON: expected object at root");
        Q_EMIT errorOccurred("Invalid JSON: expected object at root");
        return std::nullopt;
    }

    auto user = JsonService::getUserFromResponse(doc.object());
    LOG_INFO("[onGetUser] User loaded: '{}'", user.name.toStdString());
    return user;
}

int Model::getNumberOfExistingChats() const {
    LOG_INFO("[getNumberOfExistingChats] Number of chats={}", chatsById.size());
    return chatsById.size();
}

void Model::logout() {
    PROFILE_SCOPE("Model::logout");
    LOG_INFO("[logout] Logging out user");

    if(socket) {
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

void Model::clearAllChats(){
    chatsById.clear();
    LOG_INFO("[clearAllChats] clearAllChats complete");
}

void Model::clearAllMessages(){
    messageModelsByChatId.clear();
    LOG_INFO("[clearAllMessages] clearAllMessages complete");
}

QList<User> Model::findUsers(const QString& text) {
    QUrl endpoint = url_.resolved(QUrl(QString("/users/search?tag=%1").arg(text)));
    auto request = QNetworkRequest(endpoint);
    auto* reply = netManager->get(request);
    QEventLoop loop; QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); auto users = onFindUsers(reply);
    return users;
}

void Model::signMe(const QString& token){
    QUrl url("http://localhost:8083");
    QUrl endpoint = url.resolved(QUrl("/auth/me"));
    auto request = QNetworkRequest(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", token.toUtf8());
    auto* reply = netManager->get(request);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        return onSignMe(reply); });
}
QList<User> Model::onFindUsers(QNetworkReply* reply){ QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply); if (reply->error() != QNetworkReply::NoError) { Q_EMIT errorOccurred("onFindUsers" + reply->errorString()); return {}; } auto responseData = reply->readAll(); auto doc = QJsonDocument::fromJson(responseData); if (!doc.isObject()) { Q_EMIT errorOccurred("Invalid JSON: expected object at root"); return {}; } auto rootObj = doc.object(); auto arr = rootObj["users"].toArray(); QList<User> users; for (const auto& value : arr) { auto obj = value.toObject(); auto user = JsonService::getUserFromResponse(obj); users.append(user); } return users; }
void Model::addChat(const ChatPtr& chat){ PROFILE_SCOPE("Model::addChat"); chatsById[chat->chatId] = chat; chatModel->addChat(chat); Q_EMIT chatAdded(chat->chatId); }
void Model::createChat(const int chatId){ PROFILE_SCOPE("Model::createChat"); auto it = chatsById.find(chatId); if(it != chatsById.end()){ qDebug() << "[INFO] Chat " << chatId << "already exist"; return; } auto chat = loadChat(chatId); fillChatHistory(chatId); chatModel->addChatInFront(chat); }
void Model::fillChatHistory(int chatId){
    PROFILE_SCOPE("Model::fillChatHistory");
    auto messageHistory = getChatMessages(chatId);
    LOG_INFO("[fillChatHistory] For chat '{}' loaded '{}' messages", chatId, messageHistory.size());
    auto messageModel = std::make_shared<MessageModel>(this);
    messageModelsByChatId[chatId] = messageModel;

    for(auto message: messageHistory){
        auto user = getUser(message.senderId);
        if(!user) {
            LOG_ERROR("[fillChatHistory] getUser failed for message '{}'", message.id);
        }else {
            LOG_INFO("[fillChatHistory] For message '{}' user is '{}'", message.id, user->id);
        }

        messageModel->addMessageInBack(message, *user);
    }
}

ChatModel* Model::getChatModel(){
    return chatModel.get();
}
UserModel* Model::getUserModel(){
    return userModel.get();
}


/*

SELECT *
FROM messages
WHERE chat_id = 5
ORDER BY id DESC
LIMIT 20;
This returns the 20 newest messages.

b) Fetch messages older than a specific ID (beforeId)

SELECT *
FROM messages
WHERE chat_id = 5 AND id < 105
ORDER BY id DESC
LIMIT 20;


auto messages = orm->query<Message>()
                    .filter("chatId = ?", chatId)
                    .orderBy("id DESC")
                    .limit(20)
                    .execute();


*/
