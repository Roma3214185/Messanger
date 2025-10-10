#include "presenter.h"
#include <QDebug>
#include <headers/User.h>
#include "MessageModel/messagemodel.h"

Presenter::Presenter(IMainWindow* window, Model* manager)
    : view_(window)
    , manager_(manager)
{
    initialConnections();
    view_->setChatModel(manager->getChatModel());
    view_->setUserModel(manager->getUserModel());
    manager_->checkToken();
}

void Presenter::signIn(const QString& email, const QString& password){
    manager_->signIn(email, password);
}

void Presenter::signUp(const SignUpRequest& req){
    manager_->signUp(req);
}

void Presenter::initialConnections(){
    connect(manager_, &Model::userCreated, this, &Presenter::setUser);
    connect(manager_, &Model::newMessage, this, &Presenter::newMessage);
    connect(manager_, &Model::chatAdded, this, [this](int chatId){
        manager_->fillChatHistory(chatId);
    });
}

void Presenter::setUser(const User& user, const QString& token){
    view_->setUser(user);
    currentUserId_ = user.id;
    manager_->saveToken(token);

    auto chats = manager_->loadChats(); //(?) void cause it your chats history
    qDebug() << "[INFO] Presenter loaded " << chats.size() << " chats";

    for (const auto& chat : chats) {
        manager_->addChat(chat);
    }

    manager_->connectSocket(user.id);
}

void Presenter::on_chat_clicked(const int chatId){
    openChat(chatId);
}

void Presenter::newMessage(const Message& msg){
    if(currentUserId_ == msg.senderId) {
        qDebug() << "[INFO] It's your messaage" << msg.text; // впливає на те, як повідомлення розташоване, і чи оновлювати unread_message
    }

    manager_->addMessageToChat(msg.chatId, msg);
}

void Presenter::findUserRequest(const QString& text){
    if(text.isEmpty()) {
        manager_->getUserModel()->clear(); return;
    }

    auto users = manager_->findUsers(text); //make parallel request finding users and groups
    manager_->getUserModel()->clear();

    for(const auto& user: users){
        if(currentUserId_ != user.id) manager_->getUserModel()->addUser(user);
    }
}

void Presenter::openChat(const int chatId){ // make unread message = 0; (?)
    view_->setChatWindow(manager_->getMessageModel(chatId));
    currentChatId_ = chatId;
}

void Presenter::on_user_clicked(const int userId, const bool isUser /*=true (yet) */){
    manager_->getUserModel()->clear();
    view_->clearFindUserEdit();

    if(isUser && currentUserId_ == userId) {
        qDebug() << "[ERROR] Impossible to open chat with yourself";
        return;
    }

    if(isUser){
        auto chat = manager_->getPrivateChatWithUser(userId);
        if(!chat){
            qDebug() << "[ERROR] Char is null in on_user_clicked";
        }
        openChat(chat->chatId);
    }else{
        qDebug() << "[ERROR] Implement finding group request";
    }
}

void Presenter::sendButtonClicked(const QString& textToSend){
    if(textToSend.isEmpty() || !currentChatId_) {
        if(textToSend.isEmpty()) qDebug() << "[WARN] Presenter receive to send empty text";
        else qDebug() << "[ERROR] Presenter doesn't have opened chat";
        return;
    }

    manager_->sendMessage(*currentChatId_, *currentUserId_, textToSend);
}

void Presenter::on_logOutButtonClicked(){
    manager_->logout();
}



