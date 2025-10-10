#ifndef PRESENTER_H
#define PRESENTER_H

#include <QObject>
#include <optional>
#include "headers/IMainWindow.h"
#include "headers/SignUpRequest.h"
#include "Model/model.h"
#include "ChatModel/chatmodel.h"
#include "UserModel/UserModel.h"

template<typename T>
using Optional = std::optional<T>;
using OptionalInt = Optional<int>;

class Presenter : public QObject
{
    Q_OBJECT

public:

    Presenter(IMainWindow* window, Model* manager);

    void signIn(const QString& email, const QString& password);
    void signUp(const SignUpRequest& req);
    void on_chat_clicked(const int chatId);
    void findUserRequest(const QString& text);
    void on_user_clicked(const int userId, const bool isUser = true);
    void sendButtonClicked(const QString& textToSend);
    void on_logOutButtonClicked();

private:

    void initialConnections();
    void setUser(const User& user, const QString& token);
    void newMessage(const Message& message);
    void openChat(const int chatId);

    IMainWindow* view_;
    Model* manager_;
    OptionalInt currentChatId_;
    OptionalInt currentUserId_;
};

#endif // PRESENTER_H
