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

Model::Model(const QUrl& url, INetworkAccessManager* netManager, ICache* cash, QWebSocket* socket)
    : url_(url)
    , netManager(netManager)
    , cash(cash)
    , socket(socket)
    , chatModel(new ChatModel)
    , userModel(new UserModel)
{

}

void Model::checkToken(){
    auto tokenOpt = cash->get("TOKEN");
    if(tokenOpt) signMe(QString::fromStdString(*tokenOpt));
}

void Model::saveToken(const QString& token) const {
    cash->saveToken("TOKEN", token.toStdString());
}

void Model::signIn(const QString& email, const QString& password){
    QUrl endpoint = url_.resolved(QUrl("/auth/login"));
    QNetworkRequest req(endpoint);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    auto reply = netManager->post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSignInFinished(reply);
    });
}

void Model::onSignInFinished(QNetworkReply* reply){
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred(reply->errorString());
        return;
    }

    auto responseData = reply->readAll();
    auto jsonResponse = QJsonDocument::fromJson(responseData);
    auto responseObj = jsonResponse.object();

    auto createdUser = JsonService::getUserFromResponse(responseObj["user"].toObject());
    currentToken = responseObj["token"].toString();

    Q_EMIT userCreated(createdUser, currentToken);
}

void Model::signUp(const SignUpRequest& signUprequest){
    auto endpoint = url_.resolved(QUrl("/auth/register"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["email"] = signUprequest.email;
    body["password"] = signUprequest.password;
    body["name"] = signUprequest.name;
    body["tag"] = signUprequest.tag;

    auto* reply = netManager->post(request, QJsonDocument(body).toJson());

    QObject::connect(reply, &QNetworkReply::finished, this,[this, reply](){
        onSignUpFinished(reply);
    });
}

void Model::onSignUpFinished(QNetworkReply* reply){
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred(reply->errorString());
        return;
    }

    auto responseData = reply->readAll();
    auto jsonResponse = QJsonDocument::fromJson(responseData);
    auto responseObj = jsonResponse.object();

    auto createdUser = JsonService::getUserFromResponse(responseObj["user"].toObject()); // i have responseObj["user"]["tag"]
    currentToken = responseObj["token"].toString();

    Q_EMIT userCreated(createdUser, currentToken);

}

void Model::connectSocket(int id){
    connect(socket, &QWebSocket::connected, [=](){
        onSocketConnected(id);
    });

    connect(socket, &QWebSocket::textMessageReceived, this, [this](const QString& msg){
        onMessageReceived(msg);
    });

    socket->open(QUrl("ws://localhost:8082/ws"));
}

void Model::onSocketConnected(const int id){
    auto json = QJsonObject{
        {"type", "init"},
        {"userId", id}
    };

    const QString msg = QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
    socket->sendTextMessage(msg);
}

void Model::onMessageReceived(const QString& msg){
    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        Q_EMIT errorOccurred("Invalid JSON received:" + parseError.errorString());
        return;
    }

    auto obj = doc.object();
    auto newMsg = JsonService::getMessageFromJson(obj);
    Q_EMIT newMessage(newMsg);
}

ChatPtr Model::loadChat(const int chatId){
    QUrl url("http://localhost:8081");
    QUrl endpoint = url.resolved(QUrl(QString("/chats/%1").arg(chatId)));
    auto request = QNetworkRequest(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", currentToken.toUtf8());

    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto chat = onChatLoaded(reply);
    return chat;
}

ChatPtr Model::onChatLoaded(QNetworkReply* reply){
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred(reply->errorString());
        return nullptr;
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        Q_EMIT errorOccurred("loadChat: Invalid JSON — expected single chat object");
        return nullptr;
    }

    auto obj = doc.object();
    auto chat = JsonService::getChatFromJson(obj);

    return chat;
}


QList<User> Model::findUsers(const QString& text) {
    QUrl endpoint = url_.resolved(QUrl(QString("/users/search?tag=%1").arg(text)));
    auto request = QNetworkRequest(endpoint);

    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto users = onFindUsers(reply);
    return users;
}

QList<User> Model::onFindUsers(QNetworkReply* reply){
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

ChatPtr Model::createPrivateChat(int userId){
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

ChatPtr Model::onCreatePrivateChat(QNetworkReply* reply){
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred("onCreatePrivateChat" + reply->errorString());
        return nullptr;
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        Q_EMIT errorOccurred("Invalid JSON: expected object at root");
        return nullptr;
    }

    auto responseObj = doc.object();

    if(responseObj["chat_type"].toString() != "PRIVATE"){
        Q_EMIT errorOccurred("Error in model create private chat returned group chat");
        return nullptr;
    }

    auto newChat = JsonService::getPrivateChatFromJson(responseObj);
    qDebug() << "[INFO] Private chat created with id:" << newChat->chatId;
    return newChat;
}

QList<Message> Model::getChatMessages(int chatId){ //setToken and check if u can
    QUrl url("http://localhost:8082");
    QUrl endpoint = url.resolved(QUrl(QString("/messages/%1").arg(chatId)));
    auto request = QNetworkRequest(endpoint);
    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return onGetChatMessages(reply);
}

 QList<Message> Model::onGetChatMessages(QNetworkReply* reply){
     QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred("[network error] onGetChatMessages" + reply->errorString());
        return {};
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isArray()) {
        Q_EMIT("Invalid JSON: expected array at root");
        return {};
    }

    QList<Message> messages;
    for (const auto& value : doc.array()) {
        auto obj = value.toObject();
        auto msg = JsonService::getMessageFromJson(obj);
        messages.append(msg);
    }

    if(messages.size() > 0) qDebug() << "[INFO] found messages:" << messages.size() << " for chat " << messages.front().chatId;
    return messages;
}

void Model::sendMessage(const int chatId, const int senderId, const QString& textToSend) {
    if (textToSend.trimmed().isEmpty()) {
        qWarning() << "[WARN] Empty message, skipping send";
        return;
    }

    auto json = QJsonObject{
        {"type", "send_message"},
        {"sender_id", senderId},
        {"chat_id", chatId},
        {"text", textToSend},
        {"timestamp", QDateTime::currentDateTime().toString()},
    };

    const QString msg = QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
    socket->sendTextMessage(msg);
    qDebug() << "[INFO] Sending message:" << msg;
}

QList<ChatPtr> Model::loadChats(){
    QUrl url("http://localhost:8081");
    QUrl endpoint = url.resolved(QUrl(QString("/chats")));
    auto request = QNetworkRequest(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", currentToken.toUtf8());

    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return onLoadChats(reply);
}

QList<ChatPtr> Model::onLoadChats(QNetworkReply* reply){
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred("onLoadChats" + reply->errorString());
        return {};
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        Q_EMIT errorOccurred("LoadChats: Invalid JSON: expected object with 'chats' field");
        return {};
    }

    auto root = doc.object();
    if (!root.contains("chats") || !root["chats"].isArray()) {
        Q_EMIT errorOccurred("LoadChats: Invalid JSON: expected object with 'chats' field");
        return {};
    }

    auto chatArray = root["chats"].toArray();
    QList<ChatPtr> chats;

    for (const auto& value : chatArray) {
        auto newChat = JsonService::getChatFromJson(value.toObject());

        if(newChat) {
            chats.append(newChat);
        }
        else {
            Q_EMIT errorOccurred("Chat is nullptr");
            return {};
        }
    }

    qDebug() << "[info] found chats:" << chats.size();
    return chats;
}

void Model::signMe(const QString& token){
    QUrl endpoint = url_.resolved(QUrl("/auth/me"));
    auto request = QNetworkRequest(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", token.toUtf8());

    auto* reply = netManager->get(request);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        return onSignMe(reply);
    });
}

void Model::onSignMe(QNetworkReply* reply){
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[WARN] Sign me failed reply->errorString())";
        return;
    }

    auto responseData = reply->readAll();
    auto jsonResponse = QJsonDocument::fromJson(responseData);
    auto responseObj = jsonResponse.object();

    auto createdUser = JsonService::getUserFromResponse(responseObj["user"].toObject());
    currentToken = responseObj["token"].toString();
    Q_EMIT userCreated(createdUser, currentToken);
}

void Model::fillChatHistory(const int chatId){
    auto messageHistory = getChatMessages(chatId);
    auto messageModel = std::make_shared<MessageModel>(this);
    messageModelsByChatId[chatId] = messageModel;

    for(auto message: messageHistory){
        auto user = getUser(message.senderId);
        messageModel->addMessage(message, *user);
        chatModel->updateChat(chatId, message.text, message.timestamp);
    }
}

void Model::addChat(const ChatPtr& chat){
    chatsById[chat->chatId] = chat;
    chatModel->addChat(chat);
    Q_EMIT chatAdded(chat->chatId);
}

void Model::addChatInFront(const ChatPtr& chat){
    addChat(chat);
    chatModel->realocateChatInFront(chat->chatId);
}

ChatModel* Model::getChatModel(){
    return chatModel.get();
}

UserModel* Model::getUserModel(){
    return userModel.get();
}

MessageModelPtr Model::createMessageModel(const int chatId){
    auto msgModel = std::make_shared<MessageModel>();
    messageModelsByChatId[chatId] = msgModel;
    return msgModel;
}

void Model::createChat(const int chatId){
    auto it = chatsById.find(chatId);

    if(it != chatsById.end()){
        qDebug() << "[INFO] Chat " << chatId << "already exist";
        return;
    }

    auto chat = loadChat(chatId);
    fillChatHistory(chatId);
    chatModel->addChatInFront(chat);
}

void Model::addMessageToChat(const int chatId, const Message& msg){
    auto it = chatsById.find(chatId);

    if(it == chatsById.end()) {
        auto chat = loadChat(msg.chatId); // u can receive new message from group/user if u delete for youtself and from newUser
        addChatInFront(chat);
    }

    auto messageModel = messageModelsByChatId[chatId];
    auto user = getUser(msg.senderId);

    if(!user) {
        Q_EMIT errorOccurred("Server doesn't return info about user id(" + msg.senderId);
        return;
    }

    messageModel->addMessage(msg, *user);
    chatModel->updateChat(chatId, msg.text, msg.timestamp);
    chatModel->realocateChatInFront(chatId);
}

MessageModel* Model::getMessageModel(const int chatId){
    auto it = messageModelsByChatId.find(chatId);

    if(it == messageModelsByChatId.end()) createMessageModel(chatId);
    return messageModelsByChatId[chatId].get();
}

void Model::deleteToken() const {
    cash->deleteToken("TOKEN");
}

ChatPtr Model::getPrivateChatWithUser(const int userId){
    for (auto [chatId, chat] : chatsById) {
        if (chat->isPrivate()) {
            auto* pchat = static_cast<PrivateChat*>(chat.get());
            if (pchat->userId == userId) {
                qDebug() << "[INFO] Found private chat for this user:" << pchat->title;
                return chat;
            }
        }
    }

    auto chat = createPrivateChat(userId);
    addChatInFront(chat); // (!) emit chatAdded -> load chat history if exist
    return chat;
}

optional<User> Model::getUser(const int userId) {
    QUrl url("http://localhost:8083");
    QUrl endpoint = url.resolved(QUrl(QString("/users/%1").arg(userId)));
    auto request = QNetworkRequest(endpoint);

    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto user = onGetUser(reply);
    return user;
}

optional<User> Model::onGetUser(QNetworkReply* reply){
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred("get user: " + reply->errorString());
        return std::nullopt;
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        Q_EMIT errorOccurred("Invalid JSON: expected object at root");
        return std::nullopt;
    }

    return JsonService::getUserFromResponse(doc.object());
}

void Model::clearAllChats(){
    chatsById.clear();
}

void Model::clearAllMessages(){
    messageModelsByChatId.clear();
}

void Model::logout() {
    if (socket) {
        socket->disconnect();
        socket->close();
    }

    clearAllChats();
    clearAllMessages();
    deleteToken();
    currentToken.clear();
    chatModel->clear();
}

int Model::getNumberOfExistingChats() const{
    return chatsById.size();
}
