#include "model.h"

#include <QtNetwork/QNetworkRequest>
#include <QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QEventLoop>
#include <QJsonArray>
#include "headers/JsonServer.h"
#include "ChatModel/chatmodel.h"
#include "MessageModel/messagemodel.h"
#include "UserModel/UserModel.h"

Model::Model(const QUrl& url, INetworkAccessManager* netManager, ICash* cash, QWebSocket* socket)
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
    auto endpoint = url_.resolved(QUrl("/auth/login"));
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
    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred(reply->errorString());
    } else {
        auto responseData = reply->readAll();
        auto jsonResponse = QJsonDocument::fromJson(responseData);
        auto responseObj = jsonResponse.object();

        auto createdUser = JsonServer::getUserFromResponce(responseObj["user"].toObject());
        currentToken = responseObj["token"].toString();

        Q_EMIT userCreated(createdUser, currentToken);
    }

    reply->deleteLater();
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

    auto reply = netManager->post(request, QJsonDocument(body).toJson());

    QObject::connect(reply, &QNetworkReply::finished, this,[this, reply](){
        onSignUpFinished(reply);
    });
}

void Model::onSignUpFinished(QNetworkReply* reply){
    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred(reply->errorString());
    } else {
        auto responseData = reply->readAll();
        auto jsonResponse = QJsonDocument::fromJson(responseData);
        auto responseObj = jsonResponse.object();

        auto createdUser = JsonServer::getUserFromResponce(responseObj["user"].toObject()); // i have responseObj["user"]["tag"]
        currentToken = responseObj["token"].toString();

        Q_EMIT userCreated(createdUser, currentToken);
    }
    reply->deleteLater();
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
    QJsonObject json;
    json["type"] = "init";
    json["userId"] = id;

    QJsonDocument doc(json);
    auto msg = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    socket->sendTextMessage(msg);
}

void Model::onMessageReceived(const QString& msg){
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qDebug() << "[ERROR] Invalid JSON received:" << parseError.errorString();
        return;
    }

    auto obj = doc.object();
    auto newMsg = JsonServer::getMessageFromJson(obj);

    Q_EMIT newMessage(newMsg);
}

ChatPtr Model::loadChat(const int chatId){
    QUrl url("http://localhost:8081");
    auto endpoint = url.resolved(QUrl(QString("/chats/%1").arg(chatId)));
    QNetworkRequest request(endpoint);
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
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[Network error] onChatLoaded" << reply->errorString();
        reply->deleteLater();
        return nullptr;
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        qDebug() << "[ERROR] loadChat: Invalid JSON — expected single chat object";
        reply->deleteLater();
        return nullptr;
    }

    auto obj = doc.object();
    auto chat = JsonServer::getChatFromJson(obj);

    reply->deleteLater();
    return chat;
}


QList<User> Model::findUsers(const QString& text) {
    auto endpoint = url_.resolved(QUrl(QString("/users/search?tag=%1").arg(text)));
    QNetworkRequest request(endpoint);

    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto users = onFindUsers(reply);
    return users;
}

QList<User> Model::onFindUsers(QNetworkReply* reply){
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[network error] onFindUsers" << reply->errorString();
        reply->deleteLater();
        return {};
    }
    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        qWarning() << "[error] Invalid JSON: expected object at root";
        return {};
    }

    auto rootObj = doc.object();
    auto arr = rootObj["users"].toArray();

    QList<User> users;
    for (const auto& value : arr) {
        auto obj = value.toObject();
        auto user = JsonServer::getUserFromResponce(obj);
        users.append(user);
    }

    qDebug() << "[INFO] Users found:" << users.size();
    reply->deleteLater();
    return users;
}

ChatPtr Model::createPrivateChat(int userId){
    QUrl url("http://localhost:8081");
    auto endpoint = url.resolved(QUrl("/chats/private"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", currentToken.toUtf8());

    QJsonObject body;
    body["user_id"] = userId;

    auto reply = netManager->post(request, QJsonDocument(body).toJson());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto newChat = onCreatePrivateChat(reply);
    return newChat;
}

ChatPtr Model::onCreatePrivateChat(QNetworkReply* reply){
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[network error] onCreatePrivateChat" << reply->errorString();
        reply->deleteLater();
        return nullptr;
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        qWarning() << "[error] Invalid JSON: expected object at root";
        reply->deleteLater();
        return nullptr;
    }

    auto responseObj = doc.object();

    if(responseObj["chat_type"].toString() != "PRIVATE"){
        qDebug() << "Error in model create private chat returned group chat";
        reply->deleteLater();
        return nullptr;
    }

    auto newChat = JsonServer::getPrivateChatFromJson(responseObj);

    qDebug() << "[INFO] Private chat created with id:" << newChat->chatId;
    reply->deleteLater();
    return newChat;
}

QList<Message> Model::getChatMessages(int chatId){ //setToken and check if u can
    QUrl url("http://localhost:8082");
    auto endpoint = url.resolved(QUrl(QString("/messages/%1").arg(chatId)));
    QNetworkRequest request(endpoint);
    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto messages = onGetChatMessages(reply); //std::move()?
    return messages;
}

 QList<Message> Model::onGetChatMessages(QNetworkReply* reply){
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[network error] onGetChatMessages" << reply->errorString();
        return {};
    }
    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isArray()) {
        qWarning() << "[error] Invalid JSON: expected array at root";
        return {};
    }

    QList<Message> messages;
    for (const auto& value : doc.array()) {
        auto obj = value.toObject();
        auto msg = JsonServer::getMessageFromJson(obj);
        messages.append(msg);
    }

    if(messages.size() > 0) qDebug() << "[INFO] found messages:" << messages.size() << " for chat " << messages.front().chatId;
    reply->deleteLater();
    return messages;
}

void Model::sendMessage(const int chatId, const int senderId, const QString& textToSend) {
    if (textToSend.trimmed().isEmpty()) {
        qWarning() << "[WARN] Empty message, skipping send";
        return;
    }

    QJsonObject json;
    json["type"] = "send_message";
    json["sender_id"] = senderId;
    json["chat_id"] = chatId;
    json["text"] = textToSend;
    json["timestamp"] = QDateTime::currentDateTime().toString();

    QJsonDocument doc(json);
    auto msg = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    socket->sendTextMessage(msg);
}

QList<ChatPtr> Model::loadChats(){
    QUrl url("http://localhost:8081");
    auto endpoint = url.resolved(QUrl(QString("/chats")));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", currentToken.toUtf8());
    QNetworkReply* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto chats = onLoadChats(reply);
    return chats;
}

QList<ChatPtr> Model::onLoadChats(QNetworkReply* reply){

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[network error] onLoadChats" << reply->errorString();
        return {};
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);

    if (!doc.isObject()) {
        qWarning() << "[error] loadChats: Invalid JSON: expected object with 'chats' field";
        return {};
    }

    auto root = doc.object();
    if (!root.contains("chats") || !root["chats"].isArray()) {
        qWarning() << "[error] loadChats: missing or invalid 'chats' array";
        return {};
    }

    auto chatArray = root["chats"].toArray();
    QList<ChatPtr> chats;

    for (const auto& value : chatArray) {
        auto obj = value.toObject();
        auto newChat = JsonServer::getChatFromJson(obj);
        if(newChat) chats.append(newChat);
        else qDebug() << "[ERROR] Chat is nullptr";
    }

    qDebug() << "[info] found chats:" << chats.size();
    reply->deleteLater();
    return chats;
}

void Model::signMe(const QString& token){
    QUrl endpoint = url_.resolved(QUrl("/auth/me"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", token.toUtf8());

    auto* reply = netManager->get(request);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        return onSignMe(reply);
    });
}

void Model::onSignMe(QNetworkReply* reply){
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[ERROR] sign me failed";
        reply->deleteLater();
        Q_EMIT errorOccurred(reply->errorString());
        return;
    }

    auto responseData = reply->readAll();
    auto jsonResponse = QJsonDocument::fromJson(responseData);
    auto responseObj = jsonResponse.object();

    auto createdUser = JsonServer::getUserFromResponce(responseObj["user"].toObject());
    currentToken = responseObj["token"].toString();

    Q_EMIT userCreated(createdUser, currentToken);
}

void Model::fillChatHistory(const int chatId){
    qDebug() << "[INFO] start to fillChatHistory id = " << chatId;
    auto messageHistory = getChatMessages(chatId);
    qDebug() << "[INFO] fillChatHistory id = " << chatId << " messageHistory.size() = " << messageHistory.size();

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
    qDebug() << "chat wirh id was added" << chat->chatId;
    Q_EMIT(chatAdded(chat->chatId));
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
    qDebug() << "[INFO] Created new chat with id" << chatId;
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
        qDebug() << "[ERROR] I can't find info about user with id = " << msg.senderId;
        return;
    }

    //if(msg->receiver_id = *currentUserId)
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
    auto endpoint = url.resolved(QUrl(QString("/users/%1").arg(userId)));
    QNetworkRequest request(endpoint);

    auto* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto user = onGetUser(reply);
    return user;
}

optional<User> Model::onGetUser(QNetworkReply* reply){
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[network error] onGetUser" << reply->errorString();
        reply->deleteLater();
        return std::nullopt;
    }

    auto responseData = reply->readAll();
    auto doc = QJsonDocument::fromJson(responseData);
    optional<User> user;

    if (!doc.isObject()) {
        qWarning() << "[error] Invalid JSON: expected object at root";
    }else{
        auto obj = doc.object();
        user = JsonServer::getUserFromResponce(obj);
    }

    reply->deleteLater();
    return user;
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
