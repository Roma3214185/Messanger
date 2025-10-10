#ifndef PRESENTER_H
#define PRESENTER_H

#include "headers/IMainWindow.h"
#include "headers/SignUpRequest.h"
#include <Model/model.h>
#include <QObject>
#include "ChatModel/chatmodel.h"
#include "UserModel/UserModel.h"

using ChatId = int;


class Presenter : public QObject
{
    Q_OBJECT

    IMainWindow* view;
    Model* manager;
    std::optional<int> currentChatId;
    std::optional<int> currentUserId;

public:
    Presenter(IMainWindow* window, Model* manager);
    void signIn(QString email, QString password);
    void signUp(SignUpRequest req);
    void on_chat_clicked(int chatId);
    void findUserRequest(QString text);
    void on_user_clicked(int userId, bool isUser = true);
    void sendButtonClicked(QString textToSend);
    void on_logOutButtonClicked();
protected:
    void initialConnections();
    void setUser(User user, QString);
    void newMessage(Message message);
    void openChat(int newChatId);
};

#endif // PRESENTER_H
