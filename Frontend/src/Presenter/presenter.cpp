#include "presenter.h"
#include <QDebug>
#include <headers/User.h>
#include "MessageModel/messagemodel.h"
#include "../../DebugProfiling/Debug_profiling.h"
#include <QTimer>

Presenter::Presenter(IMainWindow* window, Model* manager)
    : view_(window)
    , manager_(manager)
{
    view_->setChatModel(manager->getChatModel());
    view_->setUserModel(manager->getUserModel());
    manager_->checkToken();

    messageListView = std::make_unique<MessageListView>();
    view_->setMessageListView(messageListView.get());

    initialConnections();
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
    connect(manager_, &Model::errorOccurred, this, &Presenter::onErrorOccurred);

    if(!messageListView.get()){
        LOG_ERROR("MessageListView is nullptr in initial connections");
        throw std::runtime_error("Nullptr in Presenter::connections");
    }

    connect(messageListView.get(), &MessageListView::scrollChanged, this, &Presenter::onScroll);
    connect(manager_, &Model::chatUpdated, this, &Presenter::onChatUpdated);
}

void Presenter::onChatUpdated(int chatId){
    if(!currentChatId_) return;
    QModelIndex idx = manager_->indexByChatId(chatId);
    if (idx.isValid()) {
        view_->setCurrentChatIndex(idx);
        //ui->chatListView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
    }
}

void Presenter::onScroll(int value){
    if(value != 0) return;
    PROFILE_SCOPE("Presenter::onScroll");
    int chatId = *currentChatId_;
    int oldScroll = messageListView->getMaximumMessageScrollBar() -
                    messageListView->getMessageScrollBarValue();

    auto newMessages = manager_->getChatMessages(chatId, 20);
    for(auto newMsg: newMessages){
        manager_->addMessageToChat(chatId, newMsg, false);
    }

    QTimer::singleShot(0, [this, oldScroll]() {
        messageListView->setMessageScrollBarValue(messageListView->getMaximumMessageScrollBar() - oldScroll);
    });
}

void Presenter::onErrorOccurred(const QString& error){
    view_->showError(error);
}

void Presenter::setUser(const User& user, const QString& token){
    PROFILE_SCOPE("Presenter::setUser");
    LOG_INFO("Set user name: '{}' | email '{}' | tag '{}' id '{}'", user.name.toStdString(), user.email.toStdString(), user.tag.toStdString(), user.id);

    view_->setUser(user);
    currentUserId_ = user.id;
    manager_->saveToken(token);

    auto chats = manager_->loadChats();
    LOG_INFO("In presenter loaded '{}' chats for user id '{}'", chats.size(), user.id);

    for (const auto& chat : chats) {
        manager_->addChat(chat);
    }

    manager_->connectSocket(user.id);
}

void Presenter::on_chat_clicked(const int chatId){
    openChat(chatId);
}

void Presenter::newMessage(const Message& msg){

    if(currentChatId_.has_value() && currentChatId_ == msg.chatId){
        int max = messageListView->getMaximumMessageScrollBar();
        int value = messageListView->getMessageScrollBarValue();
        manager_->addMessageToChat(msg.chatId, msg);
        LOG_INFO("In scrollBar max = '{}' and value = '{}'", max, value);
        if(max == value) messageListView->scrollToBottom();
    }else{
        manager_->addMessageToChat(msg.chatId, msg);
    }
}

void Presenter::findUserRequest(const QString& text){
    if(text.isEmpty()) {
        manager_->getUserModel()->clear(); return;
    }

    auto users = manager_->findUsers(text);
    manager_->getUserModel()->clear();

    for(const auto& user: users){
        if(currentUserId_ != user.id) manager_->getUserModel()->addUser(user);
    }
}

void Presenter::openChat(const int chatId){ // make unread message = 0; (?)
    PROFILE_SCOPE("Presenter::openChat");
    currentChatId_ = chatId;
    messageListView->setMessageModel(manager_->getMessageModel(chatId));
    messageListView->scrollToBottom();
    view_->setChatWindow();
}

void Presenter::on_user_clicked(const int userId, const bool isUser){
    manager_->getUserModel()->clear();
    view_->clearFindUserEdit();

    if(isUser && currentUserId_ == userId) {
        onErrorOccurred("[ERROR] Impossible to open chat with yourself");
        return;
    }

    if(isUser){
        auto chat = manager_->getPrivateChatWithUser(userId);
        if(!chat){
            onErrorOccurred("Char is null in on_user_clicked");
        }else{
            openChat(chat->chatId);
        }
    }else{
        qDebug() << "[ERROR] Implement finding group request";
    }
}

void Presenter::sendButtonClicked(const QString& textToSend){
    if(textToSend.isEmpty() || !currentChatId_) {
        if(textToSend.isEmpty()) qDebug() << "[WARN] Presenter receive to send empty text";
        else onErrorOccurred("Presenter doesn't have opened chat");
        return;
    }

    manager_->sendMessage(*currentChatId_, *currentUserId_, textToSend);
}

void Presenter::on_logOutButtonClicked(){
    manager_->logout();
}
