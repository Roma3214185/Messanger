#include "model.h"
#include <QtNetwork/QNetworkRequest>
#include <QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QEventLoop>
#include <QJsonArray>
#include <headers/JsonServer.h>

Model::Model(QUrl url, INetworkAccessManager* netManager, ICash* cash, QWebSocket* socket)
    : url_(url)
    , netManager(netManager)
    , cash(cash)
    , socket(socket)
    , chatModel(new ChatModel)
    , userModel(new UserModel)
{

}

void Model::checkToken(){
    std::optional<std::string> tokenOpt = cash->get("TOKEN");
    if(tokenOpt) signMe(QString::fromStdString(*tokenOpt));
}

void Model::saveToken(const QString& token) {
    cash->saveToken("TOKEN", token.toStdString());
}


void Model::signIn(QString email, QString password)
{
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
    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT errorOccurred(reply->errorString());
    } else {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = jsonResponse.object();

        auto createdUser = JsonServer::getUserFromResponce(responseObj["user"].toObject());
        auto session = responseObj["token"].toString();
        currentToken = session;

        Q_EMIT userCreated(createdUser, session);
    }

    reply->deleteLater();
}

void Model::signUp(SignUpRequest signUprequest){
    QUrl endpoint = url_.resolved(QUrl("/auth/register"));
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
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = jsonResponse.object();

        auto createdUser = JsonServer::getUserFromResponce(responseObj["user"].toObject()); // i have responseObj["user"]["tag"]
        auto session = responseObj["token"].toString();
        currentToken = session;

        Q_EMIT userCreated(createdUser, session);
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

void Model::onSocketConnected(int id){
    QJsonObject json;
    json["type"] = "init";
    json["userId"] = id;

    QJsonDocument doc(json);
    QString msg = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    socket->sendTextMessage(msg);
}


void Model::onMessageReceived(const QString& msg){
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid JSON received:" << parseError.errorString();
        return;
    }

    QJsonObject obj = doc.object();
    auto newMsg = JsonServer::getMessageFromJson(obj);

    qDebug() << "[INFO] Message timestamp = " << newMsg.timestamp;

    Q_EMIT newMessage(newMsg);
}

std::shared_ptr<ChatBase> Model::loadChat(int chatId)
{
    QUrl url("http://localhost:8081");
    QUrl endpoint = url.resolved(QUrl(QString("/chats/%1").arg(chatId)));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", currentToken.toUtf8());

    QNetworkReply* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto chat = onChatLoaded(reply);
    return chat;
}

std::shared_ptr<ChatBase> Model::onChatLoaded(QNetworkReply* reply){
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (!doc.isObject()) {
            qWarning() << "[ERROR] loadChat: Invalid JSON — expected single chat object";
            reply->deleteLater();
            return nullptr;
        }

        QJsonObject obj = doc.object();
        auto chat = JsonServer::getChatFromJson(obj);
    } else {
        qDebug() << "[Network error]" << reply->errorString();
    }

    reply->deleteLater();
    return nullptr;
}


QList<User> Model::findUsers(QString text) {
    QUrl endpoint = url_.resolved(QUrl(QString("/users/search?tag=%1").arg(text)));
    QNetworkRequest request(endpoint);
    QNetworkReply* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto users = onFindUsers(reply);
    return users;
}

QList<User> Model::onFindUsers(QNetworkReply* reply){
    QList<User> users;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (!doc.isObject()) {
            qWarning() << "[error] Invalid JSON: expected object at root";
            return users;
        }

        QJsonObject rootObj = doc.object();
        QJsonArray arr = rootObj["users"].toArray();

        for (const auto& value : arr) {
            QJsonObject obj = value.toObject();
            auto user = JsonServer::getUserFromResponce(obj);
            users.append(user);
        }
        qDebug() << "[info] users found:" << users.size();
    } else {
        qWarning() << "[network error]" << reply->errorString();
    }

    reply->deleteLater();
    return users;
}


std::shared_ptr<ChatBase> Model::createPrivateChat(int userId){
    QUrl url("http://localhost:8081");
    QUrl endpoint = url.resolved(QUrl("/chats/private"));
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

std::shared_ptr<ChatBase> Model::onCreatePrivateChat(QNetworkReply* reply){
    auto newChat = std::make_shared<PrivateChat>();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (!doc.isObject()) {
            qWarning() << "[error] Invalid JSON: expected object at root";
            return newChat;
        }

        QJsonObject responseObj = doc.object();

        if(responseObj["chat_type"].toString() != "PRIVATE"){
            qDebug() << "Error in model create private chat returned group chat";
            return newChat;
        }

        newChat->userId = responseObj["user_id"].toInt();
        newChat->chatId = responseObj["chat_id"].toInt();
        newChat->title = responseObj["title"].toString();
        newChat->avatarPath = responseObj["avatar"].toString();

        qDebug() << "[createPrivateChat] chat created with id:" << newChat->chatId;
    } else {
        qWarning() << "[network error]" << reply->errorString();
    }

    reply->deleteLater();
    return newChat;
}

QList<Message> Model::getChatMessages(int chatId){
    QUrl url("http://localhost:8082");
    QUrl endpoint = url.resolved(QUrl(QString("/messages/%1").arg(chatId)));
    QNetworkRequest request(endpoint);
    //setToken and check if u can
    QNetworkReply* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto messages = onGetChatMessages(reply);
    return messages;
}

 QList<Message> Model::onGetChatMessages(QNetworkReply* reply){
    QList<Message> messages;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (!doc.isArray()) {
            qWarning() << "[error] Invalid JSON: expected array at root";
            return messages;
        }

        for (const auto& value : doc.array()) {
            QJsonObject obj = value.toObject();
            auto msg = JsonServer::getMessageFromJson(obj);
            messages.append(msg);
        }

        qDebug() << "[info] found messages:" << messages.size();
    } else {
        qWarning() << "[network error]" << reply->errorString();
    }

    reply->deleteLater();
    return messages;
}

void Model::sendMessage(int chatId, int sender_id, const QString& textToSend) {
    if (textToSend.trimmed().isEmpty()) {
        qWarning() << "[WARN] Empty message, skipping send";
        return;
    }

    QJsonObject json;
    json["type"] = "send_message";
    json["sender_id"] = sender_id;
    json["chat_id"] = chatId;
    json["text"] = textToSend;
    json["timestamp"] = QDateTime::currentDateTime().toString();

    QJsonDocument doc(json);
    QString msg = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    socket->sendTextMessage(msg);
}

QList<std::shared_ptr<ChatBase>> Model::loadChats(){
    QUrl url("http://localhost:8081");
    QUrl endpoint = url.resolved(QUrl(QString("/chats")));
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

QList<std::shared_ptr<ChatBase>> Model::onLoadChats(QNetworkReply* reply){
    QList<std::shared_ptr<ChatBase>> chats;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (!doc.isObject()) {
            qWarning() << "[error] loadChats: Invalid JSON: expected object with 'chats' field";
            return chats;
        }

        QJsonObject root = doc.object();
        if (!root.contains("chats") || !root["chats"].isArray()) {
            qWarning() << "[error] loadChats: missing or invalid 'chats' array";
            return chats;
        }

        QJsonArray chatArray = root["chats"].toArray();
        for (const auto& value : chatArray) {

            QJsonObject obj = value.toObject();
            auto newChat = JsonServer::getChatFromJson(obj);
            if(newChat) chats.append(newChat);
            else qDebug() << "[ERROR] Chat is nullptr";
        }

        qDebug() << "[info] found chats:" << chats.size();
    } else {
        qWarning() << "[network error]" << reply->errorString();
    }

    reply->deleteLater();
    return chats;
}

void Model::signMe(QString token){
    QUrl endpoint = url_.resolved(QUrl("/auth/me"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", token.toUtf8());

    auto reply = netManager->get(request);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        return onSignMe(reply);
    });
}

void Model::onSignMe(QNetworkReply* reply){
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[ERROR] sign me failed";
        Q_EMIT errorOccurred(reply->errorString());
    } else {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = jsonResponse.object();

        auto createdUser = JsonServer::getUserFromResponce(responseObj["user"].toObject());
        auto session = responseObj["token"].toString();
        currentToken = session;

        Q_EMIT userCreated(createdUser, session);
    }
    reply->deleteLater();
}

void Model::fillChatHistory(int chatId){
    auto messageModel = std::make_shared<MessageModel>(this);
    idToMessageModel[chatId] = messageModel;

    auto messageHistory = getChatMessages(chatId);
    for(auto message: messageHistory){
        auto user = getUser(message.senderId);
        messageModel->addMessage(message, *user);
        chatModel->updateChat(chatId, message.text, message.timestamp); //fix timestamp
    }
}

void Model::addChat(std::shared_ptr<ChatBase> chat){
    existingChats[chat->chatId] = chat;
    chatModel->addChat(chat);
    Q_EMIT(chatAdded(chat->chatId));
}

void Model::addChatInFront(std::shared_ptr<ChatBase> chat){
    addChat(chat);
    chatModel->realocateChatInFront(chat->chatId);
}

ChatModel* Model::getChatModel(){
    return chatModel.get();
}
UserModel* Model::getUserModel(){
    return userModel.get();
}

std::shared_ptr<MessageModel> Model::createMessageModel(int chatId){
    auto msgModel = std::make_shared<MessageModel>();
    idToMessageModel[chatId] = msgModel;
    return msgModel;
}

void Model::createChat(int chatId){
    auto it = existingChats.find(chatId);
    if(it != existingChats.end()){
        qDebug() << "[INFO] Chat " << chatId << "already exist";
        return;
    }

    auto chat = loadChat(chatId);
    fillChatHistory(chatId);
    chatModel->addChatInFront(chat);
    qDebug() << "[INFO] Created new chat with id" << chatId;
}

void Model::addMessageToChat(int chatId, Message msg){
    auto it = existingChats.find(chatId);

    if(it == existingChats.end()) {
        auto chat = loadChat(msg.chatId); // u can receive new message from group/user if u delete for youtself and from newUser
        addChatInFront(chat);
    }

    std::shared_ptr<MessageModel> messageModel = idToMessageModel[chatId];
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

MessageModel* Model::getMessageModel(int chatId){
    auto it = idToMessageModel.find(chatId);
    if(it == idToMessageModel.end()) createMessageModel(chatId);
    return idToMessageModel[chatId].get();
}

void Model::deleteToken(){
    cash->deleteToken("TOKEN");
}

std::shared_ptr<ChatBase> Model::getPrivateChatWithUser(int userId){
    for (auto [chatId, chat] : existingChats) {
        if (chat->isPrivate()) {
            PrivateChat* pchat = static_cast<PrivateChat*>(chat.get());
            if (pchat->userId == userId) {
                qDebug() << "Found private chat for this user:" << pchat->title;
                return chat;
            }
        }
    }

    auto chat = createPrivateChat(userId);
    addChatInFront(chat); // emit chatAdded -> load chat history if exist
    return chat;
}

std::optional<User> Model::getUser(int userId) {
    QUrl url("http://localhost:8083");
    QUrl endpoint = url.resolved(QUrl(QString("/users/%1").arg(userId)));
    QNetworkRequest request(endpoint);
    QNetworkReply* reply = netManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto user = onGetUser(reply);
    return user;
}

std::optional<User> Model::onGetUser(QNetworkReply* reply){
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (!doc.isObject()) {
            qWarning() << "[error] Invalid JSON: expected object at root";
        } else {
            QJsonObject obj = doc.object();
            auto user = JsonServer::getUserFromResponce(obj);
            reply->deleteLater();
            return user;
        }
    } else {
        qDebug() << "[network error]" << reply->errorString();
    }

    reply->deleteLater();
    return std::nullopt;
}

void Model::clearAllChats(){
    existingChats.clear();
}

void Model::clearAllMessages(){
    idToMessageModel.clear();
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

int Model::getNumberOfExistingChats(){
    return existingChats.size();
}
