#include "presenter.h"
#include <QDebug>
#include <headers/User.h>
#include "MessageModel/messagemodel.h"


Presenter::Presenter(IMainWindow* window, Model* manag)
    : view(window)
    , manager(manag)
{
    initialConnections();
    view->setChatModel(manager->getChatModel());
    view->setUserModel(manager->getUserModel());
    manager->checkToken();
}

void Presenter::signIn(QString email, QString password){
    qDebug() << "[Presenter] Sign_in_user" << email << password;
    manager->signIn(email, password);

}
void Presenter::signUp(SignUpRequest req){
    qDebug() << "[Presenter]Sign_up_user" << req.email << " " << req.password << " " << req.name << " " << req.tag;
    manager->signUp(req);
}

void Presenter::initialConnections(){
    connect(manager, &Model::userCreated, this, &Presenter::setUser);
    connect(manager, &Model::newMessage, this, &Presenter::newMessage);
    connect(manager, &Model::chatAdded, this, [this](int chatId){
        manager->fillChatHistory(chatId);
    });
}

void Presenter::setUser(User user, QString token){
    view->setUser(user);
    currentUserId = user.id;
    manager->saveToken(token);

    auto chats = manager->loadChats(); //maybe void beacause it yours history's chats

    qDebug() << "[INFO] Presenter loaded " << chats.size() << " chats";

    for(auto chat: chats){
        manager->addChat(chat);
    }
    manager->connectSocket(user.id);

}

void Presenter::on_chat_clicked(int chatId){
    qDebug() << "[INFO] clicked on chat with id: " << chatId;
    openChat(chatId);
}

void Presenter::newMessage(Message msg){
    if(currentUserId == msg.senderId) {
        qDebug() << "[INFO] It's your messaage" << msg.text; // впливає на те, як повідомлення розташоване, і чи оновлювати unread_message
    }

    manager->addMessageToChat(msg.chatId, msg);
}

void Presenter::findUserRequest(QString text){
    if(text.isEmpty()) {
        manager->getUserModel()->clear();
        return;
    }

    auto users = manager->findUsers(text); //parallel request finding users and groups
    manager->getUserModel()->clear();
    for(auto user: users){
        if(currentUserId != user.id) manager->getUserModel()->addUser(user);
    }
}

void Presenter::openChat(int chatId){
    view->setChatWindow(manager->getMessageModel(chatId));
    currentChatId = chatId;

    // int idx = manager->getCurrentOpenedChatIndex()
    // view->setChatFocus(idx)
    //make unread message = 0; (?)
}

void Presenter::on_user_clicked(int userId, bool isUser /*=true (yet) */){ // click on list of finded responces by tag
    manager->getUserModel()->clear();
    view->clearFindUserEdit();

    if(isUser && currentUserId == userId) {
        qDebug() << "[ERROR] Impossible to open chat with myself ";
        return;
    }

    if(isUser){
        auto chat = manager->getPrivateChatWithUser(userId); //create if not exist
        openChat(chat->chatId);
    }else{
        qDebug() << "[ERROR] Implement finding group request";
    }
}

void Presenter::sendButtonClicked(QString textToSend){
    if(textToSend.isEmpty() || !currentChatId) {
        if(textToSend.isEmpty()) qDebug() << "[INFO] Presenter receive to send empty text";
        else qDebug() << "[INFO] Presenter doesn't have opened chat";
        return;
    }

    manager->sendMessage(*currentChatId, *currentUserId, textToSend);
}

void Presenter::on_logOutButtonClicked(){
    manager->logout();
}



